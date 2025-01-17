#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>
#include <pybind11/stl.h>
namespace py = pybind11;

#include "atom.h"
#include "aperture.h"
#include "element.h"
#include "fastlist.h"

PYBIND11_MAKE_OPAQUE(GMAD::FastList<GMAD::Atom>);

PYBIND11_MODULE(fastlist, m) {

  py::class_<GMAD::FastList<GMAD::Atom>>(m,"FastList_Atom")
    .def(py::init<>())
    .def("__iter__", [](const GMAD::FastList<GMAD::Atom> &s)
                        { return py::make_iterator(s.begin(), s.end()); },
                        py::keep_alive<0,1>());

  py::class_<GMAD::FastList<GMAD::Aperture>>(m,"FastList_Aperture")
    .def(py::init<>())
    .def("__iter__", [](const GMAD::FastList<GMAD::Aperture> &s)
    { return py::make_iterator(s.begin(), s.end()); },
    py::keep_alive<0,1>());

  py::class_<GMAD::FastList<GMAD::Element>>(m,"FastList_Element")
    .def(py::init<>())
    .def("insert",[](GMAD::FastList<GMAD::Element> &fastlist,
                     GMAD::FastList<GMAD::Element>::FastListIterator position,
                     const GMAD::Element val)
                     {fastlist.insert(position, val);})
    .def("insert",[](GMAD::FastList<GMAD::Element> &fastlist,
                     GMAD::FastList<GMAD::Element>::FastListConstIterator position,
                     const GMAD::Element val)
                     {fastlist.insert(position, val);})
    .def("insert",[](GMAD::FastList<GMAD::Element> &fastlist,
                     GMAD::FastList<GMAD::Element>::FastListIterator position,
                     GMAD::FastList<GMAD::Element>::FastListIterator first,
                     GMAD::FastList<GMAD::Element>::FastListIterator last)
                     {fastlist.insert(position, first, last);})
    .def("insert",[](GMAD::FastList<GMAD::Element> &fastlist,
                     GMAD::FastList<GMAD::Element>::FastListConstIterator position,
                     GMAD::FastList<GMAD::Element>::FastListConstIterator first,
                     GMAD::FastList<GMAD::Element>::FastListConstIterator last)
                     {fastlist.insert(position, first, last);})
    .def("insert_before",[](GMAD::FastList<GMAD::Element> &fastlist,
                            std::string& name,
                            GMAD::Element &el)
                            {fastlist.insert_before(name,el);})
    .def("push_back",[](GMAD::FastList<GMAD::Element> &fastlist,
                        GMAD::Element el,
                        bool unique=false,
                        const std::string &objectName="element")
                        { fastlist.push_back(el,unique,objectName);})
    .def("empty",&GMAD::FastList<GMAD::Element>::empty)
    .def("size",&GMAD::FastList<GMAD::Element>::size)
    .def("clear",&GMAD::FastList<GMAD::Element>::clear)
    .def("erase",[](GMAD::FastList<GMAD::Element> &fastlist){fastlist.erase();})
    // erase
    // erase
    .def("begin",[](GMAD::FastList<GMAD::Element> &fastlist) {return fastlist.begin();})
    .def("end",[](GMAD::FastList<GMAD::Element> &fastlist) {return fastlist.end();})
    .def("getVector",&GMAD::FastList<GMAD::Element>::getVector);

  py::class_<GMAD::FastList<GMAD::Element>::FastListIterator>(m,"FastList_FastListIterator")
    .def("__eq__",[](GMAD::FastList<GMAD::Element>::FastListIterator &i1,
                     GMAD::FastList<GMAD::Element>::FastListIterator &i2)
                     {return i1 == i2;});
  py::class_<GMAD::FastList<GMAD::Element>::FastListConstIterator>(m,"FastList_Element_FastListConstIterator");
  py::class_<GMAD::FastList<GMAD::Element>::FastMapIterator>(m,"FastList_Element_FastMapIterator");
  py::class_<GMAD::FastList<GMAD::Element>::FastMapConstIterator>(m,"FastList_Element_FastMapConstIterator");
  py::class_<GMAD::FastList<GMAD::Element>::FastMapIteratorPair>(m,"FastList_Element_FastMapIteratorPair");
  py::class_<GMAD::FastList<GMAD::Element>::FastMapConstIteratorPair>(m,"FastList_Element_FastMapConstIteratorPair");


}