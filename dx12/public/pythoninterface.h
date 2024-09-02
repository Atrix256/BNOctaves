#pragma once

#include "technique.h"

namespace BNOctaves
{
    inline PyObject* NoiseTypesToString(PyObject* self, PyObject* args)
    {
        int value;
        if (!PyArg_ParseTuple(args, "i:NoiseTypesToString", &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        switch((NoiseTypes)value)
        {
            case NoiseTypes::Blue: return Py_BuildValue("s", "Blue");
            case NoiseTypes::White: return Py_BuildValue("s", "White");
            case NoiseTypes::Binomial3x3: return Py_BuildValue("s", "Binomial3x3");
            case NoiseTypes::Box3x3: return Py_BuildValue("s", "Box3x3");
            case NoiseTypes::Box5x5: return Py_BuildValue("s", "Box5x5");
            case NoiseTypes::Perlin: return Py_BuildValue("s", "Perlin");
            case NoiseTypes::R2: return Py_BuildValue("s", "R2");
            case NoiseTypes::IGN: return Py_BuildValue("s", "IGN");
            default: return Py_BuildValue("s", "<invalid NoiseTypes value>");
        }
    }

    inline PyObject* Set_NoiseType(PyObject* self, PyObject* args)
    {
        int contextIndex;
        int value;

        if (!PyArg_ParseTuple(args, "ii:Set_NoiseType", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_NoiseType = (NoiseTypes)value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_NumberOfOctaves(PyObject* self, PyObject* args)
    {
        int contextIndex;
        uint value;

        if (!PyArg_ParseTuple(args, "iI:Set_NumberOfOctaves", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_NumberOfOctaves = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_DifferentNoisePerOctave(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_DifferentNoisePerOctave", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_DifferentNoisePerOctave = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_RNGSeed(PyObject* self, PyObject* args)
    {
        int contextIndex;
        uint value;

        if (!PyArg_ParseTuple(args, "iI:Set_RNGSeed", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_RNGSeed = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_PerlinCellSize(PyObject* self, PyObject* args)
    {
        int contextIndex;
        uint value;

        if (!PyArg_ParseTuple(args, "iI:Set_PerlinCellSize", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_PerlinCellSize = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_PerlinMinMax(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float2 value;

        if (!PyArg_ParseTuple(args, "iff:Set_PerlinMinMax", &contextIndex, &value[0], &value[1]))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_PerlinMinMax = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_Histogram_NumBuckets(PyObject* self, PyObject* args)
    {
        int contextIndex;
        uint value;

        if (!PyArg_ParseTuple(args, "iI:Set_Histogram_NumBuckets", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Histogram_NumBuckets = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_Histogram_GraphSize(PyObject* self, PyObject* args)
    {
        int contextIndex;
        uint2 value;

        if (!PyArg_ParseTuple(args, "iII:Set_Histogram_GraphSize", &contextIndex, &value[0], &value[1]))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Histogram_GraphSize = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_Histogram_XAxisRange(PyObject* self, PyObject* args)
    {
        int contextIndex;
        float2 value;

        if (!PyArg_ParseTuple(args, "iff:Set_Histogram_XAxisRange", &contextIndex, &value[0], &value[1]))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Histogram_XAxisRange = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_Histogram_AutoXAxisRange(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_Histogram_AutoXAxisRange", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Histogram_AutoXAxisRange = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_Histogram_ZeroMinMaxBucket(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_Histogram_ZeroMinMaxBucket", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_Histogram_ZeroMinMaxBucket = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_DFT_RemoveDC(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_DFT_RemoveDC", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_DFT_RemoveDC = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    inline PyObject* Set_DFT_LogSpaceMagnitude(PyObject* self, PyObject* args)
    {
        int contextIndex;
        bool value;

        if (!PyArg_ParseTuple(args, "ib:Set_DFT_LogSpaceMagnitude", &contextIndex, &value))
            return PyErr_Format(PyExc_TypeError, "type error");

        Context* context = Context::GetContext(contextIndex);
        if (!context)
            return PyErr_Format(PyExc_IndexError, __FUNCTION__, "() : index % i is out of range(count = % i)", contextIndex, Context::GetContextCount());

        context->m_input.variable_DFT_LogSpaceMagnitude = value;

        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyMethodDef pythonModuleMethods[] = {
        {"NoiseTypesToString", NoiseTypesToString, METH_VARARGS, ""},
        {"Set_NoiseType", Set_NoiseType, METH_VARARGS, "The type of noise to use"},
        {"Set_NumberOfOctaves", Set_NumberOfOctaves, METH_VARARGS, "How many octaves to use"},
        {"Set_DifferentNoisePerOctave", Set_DifferentNoisePerOctave, METH_VARARGS, "If false, the same noise will be used for each octave. If true, a different noise, of the same type, will be used for each octave."},
        {"Set_RNGSeed", Set_RNGSeed, METH_VARARGS, "A PRNG is used for various things, change this value to change thats eed."},
        {"Set_PerlinCellSize", Set_PerlinCellSize, METH_VARARGS, ""},
        {"Set_PerlinMinMax", Set_PerlinMinMax, METH_VARARGS, "Perlin noise can go below zero which causes problems in this demo. To help that, this is the range of values which are mapped to [0,1]. Anything lower than 0 is clipped to 0 after the remapping."},
        {"Set_Histogram_NumBuckets", Set_Histogram_NumBuckets, METH_VARARGS, ""},
        {"Set_Histogram_GraphSize", Set_Histogram_GraphSize, METH_VARARGS, ""},
        {"Set_Histogram_XAxisRange", Set_Histogram_XAxisRange, METH_VARARGS, ""},
        {"Set_Histogram_AutoXAxisRange", Set_Histogram_AutoXAxisRange, METH_VARARGS, ""},
        {"Set_Histogram_ZeroMinMaxBucket", Set_Histogram_ZeroMinMaxBucket, METH_VARARGS, "If values are clamped to a min and max value, the min and max bucket will have too many counts in them. This option zeros them out to make the rest of the data easier to see."},
        {"Set_DFT_RemoveDC", Set_DFT_RemoveDC, METH_VARARGS, "DC (0hz) is often a large spike that makes it hard to see the rest of the frequencies. Use this to set DC to zero."},
        {"Set_DFT_LogSpaceMagnitude", Set_DFT_LogSpaceMagnitude, METH_VARARGS, "If true, show magnitude in log space"},
        {nullptr, nullptr, 0, nullptr}
    };

    static PyModuleDef pythonModule = {
        PyModuleDef_HEAD_INIT, "BNOctaves", NULL, -1, pythonModuleMethods,
        NULL, NULL, NULL, NULL
    };

    PyObject* CreateModule()
    {
        PyObject* module = PyModule_Create(&pythonModule);
        PyModule_AddIntConstant(module, "NoiseTypes_Blue", 0);
        PyModule_AddIntConstant(module, "NoiseTypes_White", 1);
        PyModule_AddIntConstant(module, "NoiseTypes_Binomial3x3", 2);
        PyModule_AddIntConstant(module, "NoiseTypes_Box3x3", 3);
        PyModule_AddIntConstant(module, "NoiseTypes_Box5x5", 4);
        PyModule_AddIntConstant(module, "NoiseTypes_Perlin", 5);
        PyModule_AddIntConstant(module, "NoiseTypes_R2", 6);
        PyModule_AddIntConstant(module, "NoiseTypes_IGN", 7);
        return module;
    }
};
