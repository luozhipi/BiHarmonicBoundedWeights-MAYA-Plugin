#http://help.autodesk.com/view/MAYAUL/2016/ENU/?guid=__cpp_ref_index_html
__author__ = "Zhiping Luo"
__contact__ = "luozhipi@gmail.com"
__website__ = "luozhipi.github.io"
#----------------------------------------------------------------------
import wingdbstub
def launch():
    """"""
    from startup import setup
    reload(setup)
    setup.launch()