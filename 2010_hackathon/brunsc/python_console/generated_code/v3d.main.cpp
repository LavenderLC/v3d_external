// This file has been generated by Py++.

#include "boost/python.hpp"

#include "__convenience.pypp.hpp"

#include "__call_policies.pypp.hpp"

#include "wrappable_v3d.h"

#include "generated_code/Image4DSimple.pypp.hpp"

#include "generated_code/ImageWindow.pypp.hpp"

#include "generated_code/LocationSimple.pypp.hpp"

#include "generated_code/NeuronSWC.pypp.hpp"

#include "generated_code/NeuronTree.pypp.hpp"

#include "generated_code/QPoint.pypp.hpp"

#include "generated_code/QPolygon.pypp.hpp"

#include "generated_code/QRect.pypp.hpp"

#include "generated_code/QSize.pypp.hpp"

#include "generated_code/QVector_QPoint.pypp.hpp"

#include "generated_code/QWidget.pypp.hpp"

#include "generated_code/RGB16i.pypp.hpp"

#include "generated_code/RGB32f.pypp.hpp"

#include "generated_code/RGB32i.pypp.hpp"

#include "generated_code/RGB8.pypp.hpp"

#include "generated_code/RGBA8.pypp.hpp"

#include "generated_code/TriviewControl.pypp.hpp"

#include "generated_code/V3D_GlobalSetting.pypp.hpp"

#include "generated_code/View3DControl.pypp.hpp"

#include "generated_code/XYZ.pypp.hpp"

#include "generated_code/c_array_float_3.pypp.hpp"

#include "generated_code/c_array_int_3.pypp.hpp"

#include "generated_code/c_array_short_3.pypp.hpp"

#include "generated_code/c_array_uint_3.pypp.hpp"

#include "generated_code/c_array_uint_4.pypp.hpp"

#include "generated_code/v3d_enumerations.pypp.hpp"

#include "generated_code/v3d_free_functions.pypp.hpp"

namespace bp = boost::python;

#include "convert_qlist.h"

#include "convert_qstring.h"

#include "convert_qbool.h"

#include "convert_qhash.h"

#include "convert_c_array_struct.h"

BOOST_PYTHON_MODULE(v3d){
    register_enumerations();

    register_c_array_struct_conversion< c_array<short, 3> >();

    register_c_array_struct_conversion< c_array<float, 3> >();

    register_c_array_struct_conversion< c_array<unsigned char, 4> >();

    register_c_array_struct_conversion< c_array<int, 3> >();

    register_c_array_struct_conversion< c_array<unsigned char, 3> >();

    register_qhash_conversion< QHash<int, int> >();

    register_qbool_conversion();

    register_qstring_conversion();

    register_qlist_conversion<QList<QPolygon> >();

    register_qlist_conversion<QList<NeuronSWC> >();

    register_qlist_conversion<QList<LocationSimple> >();

    register_qlist_conversion<QVector<QPoint> >();

    register_Image4DSimple_class();

    register_ImageWindow_class();

    register_LocationSimple_class();

    register_NeuronSWC_class();

    bp::implicitly_convertible< NeuronSWC, XYZ >();

    register_NeuronTree_class();

    register_QPoint_class();

    register_QVector_QPoint_class();

    register_QPolygon_class();

    register_QRect_class();

    register_QSize_class();

    register_QWidget_class();

    register_RGB16i_class();

    register_RGB32f_class();

    register_RGB32i_class();

    register_RGB8_class();

    register_RGBA8_class();

    register_TriviewControl_class();

    register_V3D_GlobalSetting_class();

    register_View3DControl_class();

    register_XYZ_class();

    register_c_array_float_3_class();

    register_c_array_int_3_class();

    register_c_array_short_3_class();

    register_c_array_uint_3_class();

    register_c_array_uint_4_class();

    boost::python::scope().attr("__doc__") = 
                "Python module for interacting with the V3D visualization program."
                "\n\n"
                "The v3d python module enables control of the "
                "V3D biomedical volume visualization program "
                "from the python programming language.  The v3d "
                "module exposes the V3D plug-in API; thus most "
                "things that can be done from a V3D plug-in can also be "
                "done interactively from the V3D python console."
                ;

    register_free_functions();
}

