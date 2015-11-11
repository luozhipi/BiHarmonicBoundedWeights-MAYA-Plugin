#ifndef UTILS_HPP
#define UTILS_HPP

#include "MAYA_inc.h"
#include "EIGEN_inc.h"

/* stl set operation wrappers */

template<typename T>
set<T> set_union( const set<T> &a, const set<T> &b )
{
    set<T> tmp;
    std::set_union( a.begin(), a.end(), b.begin(), b.end(), inserter( tmp, tmp.begin() ) );
    return tmp;
}

template<typename T>
set<T> set_difference( const set<T> &a, const set<T> &b )
{
    set<T> tmp;
    std::set_difference( a.begin(), a.end(), b.begin(), b.end(), inserter( tmp, tmp.begin() ) );
    return tmp;
}

template<typename T>
set<T> set_intersection( const set<T> &a, const set<T> &b )
{
    set<T> tmp;
    std::set_intersection( a.begin(), a.end(), b.begin(), b.end(), inserter( tmp, tmp.begin() ) );
    return tmp;
}

template<typename T>
set<T> pickKSampleSet( const vector<T> &v, unsigned int K )
{
    assert( K <= v.size() );

    set<T> result;
    random_device rd;
    mt19937_64 rng( rd() );
    uniform_int_distribution<int> uni( 0, (int)v.size() - 1 );

    while ( result.size() < K ) {
        int i = uni( rng );
        T val = v[i];
        if ( !result.count( val ) ) result.insert( val );
    }
    return result;
}

/* The following functions are utilities for maya */
MObject getDependNode( const MString &name )
{
    MSelectionList selList;
    selList.add( name );
    MObject depNode;
    selList.getDependNode( 0, depNode );
    return depNode;
}

MDagPath getDagPath( const MString &name )
{
    MSelectionList selList;
    selList.add( name );
    MDagPath dagPath;
    selList.getDagPath( 0, dagPath );
    return dagPath;
}

MObjectArray getAllDGNodes( const MObject& node,
                            MFn::Type type,
                            MItDependencyGraph::Direction direction,
                            MStatus* ReturnStatus=NULL )
{
    MObjectArray result;

    // Create a dependency graph iterator for our current object:
    MObject tmpNode(node);
    MItDependencyGraph mItDependencyGraph( tmpNode,
                                           type,
										   direction,
    		                               MItDependencyGraph::kDepthFirst,
    		                               MItDependencyGraph::kNodeLevel,
    		                               ReturnStatus);
    //CHECK_MSTATUS( *ReturnStatus );

    for ( mItDependencyGraph.reset();
          !mItDependencyGraph.isDone( ReturnStatus );
          mItDependencyGraph.next() ) {
        MObject currItem = mItDependencyGraph.currentItem( ReturnStatus );
        CHECK_MSTATUS( *ReturnStatus );

        *ReturnStatus = result.append( currItem );
        CHECK_MSTATUS( *ReturnStatus );
    }
    return result;
}

MObject getSkinNode( const MDagPath& shapePath, MStatus *ReturnStatus=NULL )
{
    MObjectArray result = getAllDGNodes( shapePath.node(),
                                         MFn::kSkinClusterFilter,
                                         MItDependencyGraph::kUpstream,
                                         ReturnStatus );
    CHECK_MSTATUS( *ReturnStatus );

    if ( result.length() == 0 ) return MObject();
    return result[0];
}

MStatus setCurrent( int t )
{
    MStatus stat;
    MAnimControl animCtrl;
    MTime mt( (double)t );
    stat = animCtrl.setCurrentTime( mt );
    return stat;
}

MStringArray retrieveStringArrayFromMultiFlag( const MArgDatabase &argData,
                                               const MString &flag )
{
    MStatus stat;
    MArgList args;
    MStringArray strArray;
    int i = 0;

    while ( true ) {
        MString itemName;
        stat = argData.getFlagArgumentList( flag.asChar(), i, args );
        if ( stat == MS::kSuccess ) {
            itemName = args.asString( i );
            strArray.append( itemName );
        }
        else {
            break;
        }
        i++;
    }
    return strArray;
}
#endif
