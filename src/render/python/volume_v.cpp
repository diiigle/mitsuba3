#include <mitsuba/render/volume.h>
#include <mitsuba/python/python.h>
#include <mitsuba/core/properties.h>

/// Trampoline for derived types implemented in Python
MI_VARIANT class PyVolume : public Volume<Float, Spectrum> {
public:
    MI_IMPORT_TYPES(Volume)

    PyVolume(const Properties &props) : Volume(props) { };

    UnpolarizedSpectrum eval(const Interaction3f &it,
                             Mask active = true) const override {
        PYBIND11_OVERRIDE_PURE(UnpolarizedSpectrum, Volume, eval, it, active);
    }

    Float eval_1(const Interaction3f &it, Mask active = true) const override {
        PYBIND11_OVERRIDE_PURE(Float, Volume, eval_1, it, active);
    }

    Vector3f eval_3(const Interaction3f &it,
                    Mask active = true) const override {
        PYBIND11_OVERRIDE_PURE(Vector3f, Volume, eval_3, it, active);
    }

    dr::Array<Float, 6> eval_6(const Interaction3f &it,
                               Mask active = true) const override {
        using Return = dr::Array<Float, 6>;
        PYBIND11_OVERRIDE_PURE(Return, Volume, eval_6, it, active);
    }

    std::pair<UnpolarizedSpectrum, Vector3f>
    eval_gradient(const Interaction3f &it, Mask active = true) const override {
        using Return = std::pair<UnpolarizedSpectrum, Vector3f>;
        PYBIND11_OVERRIDE_PURE(Return, Volume, eval_gradient, it, active);
    }

    ScalarFloat max() const override {
        PYBIND11_OVERRIDE_PURE(ScalarFloat, Volume, max);
    }

    ScalarVector3i resolution() const override {
        PYBIND11_OVERRIDE(ScalarVector3i, Volume, resolution);
    }

    std::string to_string() const override {
        PYBIND11_OVERRIDE(std::string, Volume, to_string);
    }
};

template <typename Ptr, typename Cls> void bind_volume_generic(Cls &cls) {
    // cls.def_method(Volume, resolution)
    //     .def_method(Volume, bbox)
    //     .def_method(Volume, channel_count)

    cls.def(
           "eval",
           [](Ptr ptr, const Interaction3f &it, Mask active) {
               return ptr->eval(it, active);
           },
           "it"_a, "active"_a = true, D(Volume, eval))
        .def(
            "eval_1",
            [](Ptr ptr, const Interaction3f &it, Mask active) {
                return ptr->eval_1(it, active);
            },
            "it"_a, "active"_a = true, D(Volume, eval_1))
        .def(
            "eval_3",
            [](Ptr ptr, const Interaction3f &it, Mask active) {
                return ptr->eval_3(it, active);
            },
            "it"_a, "active"_a = true, D(Volume, eval_3))
        .def(
            "eval_6",
            [](Ptr ptr, const Interaction3f &it, Mask active) {
                return ptr->eval_6(it, active);
            },
            "it"_a, "active"_a = true, D(Volume, eval_6))
        .def(
            "eval_gradient",
            [](Ptr ptr, const Interaction3f &it, Mask active) {
                return ptr->eval_gradient(it, active);
            },
            "it"_a, "active"_a = true, D(Volume, eval_gradient))
        .def(
           "max",
           [](Ptr ptr) {
               return ptr->max();
           },
           D(Volume, max));
    
    if constexpr (dr::is_array_v<Ptr>)
        bind_drjit_ptr_array(cls);
}

MI_PY_EXPORT(Volume) {
    MI_PY_IMPORT_TYPES(Volume, VolumePtr)
    using PyVolume = PyVolume<Float, Spectrum>;

    auto volume = MI_PY_TRAMPOLINE_CLASS(PyVolume, Volume, Object)
        .def(py::init<const Properties &>(), "props"_a)
        .def("max_per_channel",
            [] (const Volume *volume) {
                std::vector<ScalarFloat> max_values(volume->channel_count());
                volume->max_per_channel(max_values.data());
                return max_values;
            },
            D(Volume, max_per_channel))
        .def("eval_6",
            [](const Volume &volume, const Interaction3f &it, const Mask active) {
                dr::Array<Float, 6> result = volume.eval_6(it, active);
                std::array<Float, 6> output;
                for (size_t i = 0; i < 6; ++i)
                    output[i] = std::move(result[i]);
                return output;
            }, "it"_a, "active"_a = true, D(Volume, eval_6))
        .def("eval_n",
            [] (const Volume *volume, const Interaction3f &it, Mask active = true) {
                std::vector<Float> evaluation(volume->channel_count());
                volume->eval_n(it, evaluation.data(), active);
                return evaluation;
            },
            "it"_a, "active"_a = true,
            D(Volume, eval_n));
        .def("__repr__", &Volume::to_string);


    bind_volume_generic<Volume *>(volume);

    if constexpr (dr::is_array_v<VolumePtr>) {
        py::object dr       = py::module_::import("drjit"),
                   dr_array = dr.attr("ArrayBase");

        py::class_<VolumePtr> cls(m, "VolumePtr", dr_array);
        bind_volume_generic<VolumePtr>(cls);
    }

    MI_PY_REGISTER_OBJECT("register_volume", Volume)
}
