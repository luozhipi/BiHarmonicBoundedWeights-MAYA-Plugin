#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "GL_inc.h"
#include "MAYA_inc.h"
#include "Array3D.h"
#include "STL_inc.h"
#include "BoxGrid.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void compute(BoxGrid& voxGrid, map<string, RowVector3> B, map<string, string> boneWise)
{
	voxGrid.computeBoneBBW(B, boneWise);
}

bool Voxelize(const MFnMesh& mesh, int resX, int resY, int resZ, MPointArray& voxels, Array3D<int>& m_voxArray)
{
	// This method is an implementation of the paper "Single-Pass GPU Solid 
	// Voxelization for Real-Time Applications"
	// http://hal.inria.fr/docs/00/34/52/91/PDF/solidvoxelizationAuthorVersion.pdf
	//
	// The idea is to render the provided mesh through an orthographic view fitted
	// around the bounding box and, for each fragment, packing the depth information
	// on the color components, and accumulating the results in the framebuffer using
	// a XOR bitwise operator in the blend mode. The resulting image will contain
	// a row of voxels for each x,y pixel, packed in the color bits. To reconstruct
	// the voxels from there, we just have to check which bits are set to 1 and
	// dump a voxel (given as a pair of min/max points) to the output array.

	// Note the implementation of this method is self-contained and therefore
	// we're allocating and deallocating resources each time we voxelize. This
	// should be refactored in case of a continuous voxelization.

	glewInit();

	// clamp to the limits of this implementation 
	// (128 bits as 4 x 32 bit color channels)
	resX = max(1, min(128, resX));
	resY = max(1, min(128, resY));
	resZ = max(1, min(128, resZ));

	struct Vertex {
		float x, y, z, w;
	};
	typedef GLuint Index;
	int numIndices = 0;

	// create resources
	GLuint vbo, ibo;
	GLuint renderTarget;
	GLuint program, vs, fs;
	GLuint bitmaskTex;
	GLuint fbo, rbo;

	MBoundingBox bounds;

	{ // create shader program

		{ // Vertex shader
			vs = glCreateShader(GL_VERTEX_SHADER);

			const char* shader = "varying float depth; \r\n\
								 								 								 uniform float nearClipPlane;\r\n\
																								 																 								 uniform float farClipPlane;\r\n\
																																																 																								 								 void main() { \r\n\
																																																																																 																																 									gl_Position = ftransform(); \r\n\
																																																																																																																																																																											vec4 transformed = gl_ModelViewMatrix * gl_Vertex;\r\n\
																																																																																																																																																																																																																																						depth = (-transformed.z / transformed.w ) / ( farClipPlane - nearClipPlane );\r\n\
																																																																																																																																																																																																																																																																																																									 }";
			glShaderSource(vs, 1, &shader, NULL);
			glCompileShader(vs);

			{
				char log[512];
				GLsizei loglength;
				glGetShaderInfoLog(vs, 512, &loglength, log);
				if (loglength > 0) {
					cerr << "OpenGL shader compiler: " << log << endl;
				}
			}
		}

		{ // pixel shader

			fs = glCreateShader(GL_FRAGMENT_SHADER);

			const char* shader = "uniform sampler1D bitmask; \r\n\
								 								 								 varying float depth; \r\n\
																								 																 								 void main() { \r\n\
																																																 																								 									gl_FragColor = texture1D( bitmask, depth );\r\n\
																																																																																																																										  }";
			glShaderSource(fs, 1, &shader, NULL);
			glCompileShader(fs);

			{
				char log[512];
				GLsizei loglength;
				glGetShaderInfoLog(fs, 512, &loglength, log);
				if (loglength > 0) {
					cerr << "OpenGL shader compiler: " << log << endl;
				}
			}
		}

		program = glCreateProgram();
		glAttachShader(program, vs);
		glAttachShader(program, fs);

		glLinkProgram(program);
	}

	{ // Generate destination texture and frame buffer object

		glGenTextures(1, &renderTarget);
		glBindTexture(GL_TEXTURE_2D, renderTarget);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, resX, resY, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);

		// create a framebuffer object, you need to delete them when program exits.
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		// create a renderbuffer object to store depth info
		// NOTE: A depth renderable image should be attached the FBO for depth test.
		// If we don't attach a depth renderable image to the FBO, then
		// the rendering output will be corrupted because of missing depth test.
		// If you also need stencil test for your rendering, then you must
		// attach additional image to the stencil attachement point, too.
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, resX, resY);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// attach a texture to FBO color attachment point
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget, 0);

		// attach a renderbuffer to depth attachment point
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);


		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE){
			cerr << "error setting up frame buffer object" << endl;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// map Maya mesh to OpenGL
	{
		{ // copy vertices

			glGenBuffers(1, &vbo);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, mesh.numVertices() * sizeof(Vertex), NULL, GL_STATIC_DRAW);
			Vertex* glVertices = (Vertex*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
			if (glVertices == NULL) return false;

			MFloatPointArray vertices;
			mesh.getPoints(vertices, MSpace::kWorld);

			bounds.clear();

			for (unsigned int i = 0; i < vertices.length(); i++) {
				MFloatPoint& worldPoint = vertices[i];
				glVertices[i].x = worldPoint.x;
				glVertices[i].y = worldPoint.y;
				glVertices[i].z = worldPoint.z;
				glVertices[i].w = worldPoint.w;
				bounds.expand(worldPoint);
			}

			// commit data
			glUnmapBuffer(GL_ARRAY_BUFFER);

		}

		{ // copy indices
			MIntArray triangleCounts, triVertices;
			mesh.getTriangles(triangleCounts, triVertices);

			glGenBuffers(1, &ibo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
			numIndices = (int)triVertices.length();
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * 3 * sizeof(Index), NULL, GL_STATIC_DRAW);
			Index* indices = (Index*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

			for (unsigned int i = 0; i < triVertices.length(); i++) {
				indices[i] = triVertices[i];
			}

			// commit data
			glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		}
	}

	{ // create bit mask lookup texture for the fragment shader

		GLuint* lookup = (GLuint*)calloc(4 * resZ, sizeof(GLuint));
		for (int i = 1; i < resZ; i++) {
			if (i < 32) {
				lookup[4 * i + 3] = (1U << min(32, i)) - 1;
			}
			else if (i == 32) {
				lookup[4 * i + 3] = 0xFFFFFFFF;
			}
			else if (i < 64) {
				lookup[4 * i + 3] = 0xFFFFFFFF;
				lookup[4 * i + 2] = (1U << min(32, (i - 32))) - 1;
			}
			else if (i == 64) {
				lookup[4 * i + 3] = 0xFFFFFFFF;
				lookup[4 * i + 2] = 0xFFFFFFFF;
			}
			else if (i < 96) {
				lookup[4 * i + 3] = 0xFFFFFFFF;
				lookup[4 * i + 2] = 0xFFFFFFFF;
				lookup[4 * i + 1] = (1U << min(32, (i - 64))) - 1;
			}
			else if (i == 96) {
				lookup[4 * i + 3] = 0xFFFFFFFF;
				lookup[4 * i + 2] = 0xFFFFFFFF;
				lookup[4 * i + 1] = 0xFFFFFFFF;
			}
			else {
				lookup[4 * i + 3] = 0xFFFFFFFF;
				lookup[4 * i + 2] = 0xFFFFFFFF;
				lookup[4 * i + 1] = 0xFFFFFFFF;
				lookup[4 * i + 0] = (1U << min(32, (i - 96))) - 1;
			}
		}

		glClampColorARB(GL_CLAMP_VERTEX_COLOR_ARB, GL_FALSE);
		glClampColorARB(GL_CLAMP_FRAGMENT_COLOR_ARB, GL_FALSE);
		glClampColorARB(GL_CLAMP_READ_COLOR_ARB, GL_FALSE);


		glGenTextures(1, &bitmaskTex);
		glBindTexture(GL_TEXTURE_1D, bitmaskTex);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32UI, resZ, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, lookup);
		free(lookup);

		GLenum errCode;
		const GLubyte *errString;

		if ((errCode = glGetError()) != GL_NO_ERROR) {
			errString = gluErrorString(errCode);
			fprintf(stderr, "OpenGL Error: %s\n", errString);
		}
	}


	{ // setup modelView/projection matrices

		const float epsilon = 1e-2f;

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(-bounds.width() / 2, bounds.width() / 2,
			-bounds.height() / 2, bounds.height() / 2,
			0, bounds.max().z - bounds.min().z);
		/*bounds.min().z, bounds.max().z );*/

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		//glScaled( 1.0 / bounds.width(), 1.0 / bounds.height(), 1.0 / bounds.depth() );
		gluLookAt(bounds.center().x, bounds.center().y, bounds.max().z,
			bounds.center().x, bounds.center().y, bounds.center().z,
			0, 1, 0);
	}

	// Render ////////////////////////////////////////////////////////////////////////

	// set state and shader input variables
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(4, GL_FLOAT, 0, BUFFER_OFFSET(0));
	glEnableClientState(GL_INDEX_ARRAY);
	glIndexPointer(GL_INT, 0, NULL);
	glEnable(GL_TEXTURE_1D);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);				// Set The Clear Color To Medium Blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And Depth Buffer

	glDisable(GL_DEPTH_TEST);

	glUseProgram(program);

	// feed shader variables

	int bitmaskHandler = glGetUniformLocation(program, "bitmask");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, bitmaskTex);
	glUniform1i(bitmaskHandler, 0);

	int nearClipHandle = glGetUniformLocation(program, "nearClipPlane");
	glUniform1f(nearClipHandle, 0.0f);

	int farClipHandle = glGetUniformLocation(program, "farClipPlane");
	glUniform1f(farClipHandle, (float)bounds.depth());

	// set blending mode
	glLogicOp(GL_XOR);
	glEnable(GL_COLOR_LOGIC_OP);

	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, resX, resY);

	// render geometry
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, NULL);

	glPopAttrib();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLenum errCode;
	const GLubyte *errString;

	if ((errCode = glGetError()) != GL_NO_ERROR) {
		errString = gluErrorString(errCode);
		fprintf(stderr, "OpenGL Error: %s\n", errString);
	}

	// gather the resulting texture data
	glBindTexture(GL_TEXTURE_2D, renderTarget);
	unsigned int* data = (unsigned int*)malloc(4 * resX * resY * sizeof(unsigned int));
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, data);

	if ((errCode = glGetError()) != GL_NO_ERROR) {
		errString = gluErrorString(errCode);
		fprintf(stderr, "OpenGL Error: %s\n", errString);
	}

	{ // dump voxels

		voxels.clear();

		const float deltaX = (float)bounds.width() / resX;
		const float deltaY = (float)bounds.height() / resY;
		const float deltaZ = (float)bounds.depth() / resZ;

		// due to the way the bits are constructed, the
		// resulting boxes are shifted half a voxel - we'll
		// compensate that shift while dumping the geometry.
		const MPoint halfVoxel(0, 0, -deltaZ * 0.5f);

		for (int y = 0; y < resY; y++) {
			for (int x = 0; x < resX; x++) {

				for (int i = 3; i >= 0; i--) {
					unsigned int col = data[4 * (x + y * resX) + i];

					if (col == 0) continue;

					for (int z = 0; z < 32; z++) { // unpack color data

						if ((col & (1 << z)) != 0) { // if the z-th bit is set, create a voxel

							MPoint bbMin(bounds.min().x + x * deltaX, bounds.min().y + y * deltaY, bounds.max().z - (32 * (3 - i) + z) * deltaZ);
							MPoint bbMax(bbMin.x + deltaX, bbMin.y + deltaY, bbMin.z + deltaZ);
							bbMin += halfVoxel;
							bbMax += halfVoxel;
							voxels.append(bbMin);
							voxels.append(bbMax);
							m_voxArray(x, y, resZ - (32 * (3 - i) + z)) = 1;
						}
					}
				}
			}
		}
	}

	free(data);

	// restore state


	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_INDEX_ARRAY);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glUseProgram(0);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_COLOR_LOGIC_OP);

	// dispose resources /////////////////////////////////////////////////////////////

	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ibo);
	glDeleteTextures(1, &renderTarget);
	glDeleteShader(fs);
	glDeleteProgram(program);
	glDeleteFramebuffers(1, &fbo);
	glDeleteRenderbuffers(1, &rbo);

	return true;
}

void UnitPacking(const MFnMesh& fnMesh, PointMatrixType& vertices, RowVector3& bmin, RowVector3& bmax,
	ScalarType& scale, RowVector3& center)
{
	MPointArray points;
	fnMesh.getPoints(points, MSpace::kWorld);
	for (unsigned int i = 0; i < points.length(); i++)
	{
		RowVector3 v(points[i].x, points[i].y, points[i].z);
		vertices.row(i) = v;
	}
	size_t nbV = vertices.rows();
	bmin = bmax = vertices.row(0);
	for (int i = 1; i < nbV; i++) {
		bmin = bmin.cwiseMin(vertices.row(i));
		bmax = bmax.cwiseMax(vertices.row(i));
	}
	RowVector3 length = (bmax - bmin).cwiseInverse();
	ScalarType min_scale = 0.95*length.minCoeff();
	scale = min_scale;
	center = (bmin + bmax) / 2.0;
	for (int i = 0; i < nbV; i++) {
		vertices.row(i) = min_scale*(vertices.row(i) - center) + RowVector3(0.5, 0.5, 0.5);
	}
}

#endif