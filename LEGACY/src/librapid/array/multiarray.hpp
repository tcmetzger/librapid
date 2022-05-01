#ifndef LIBRAPID_ARRAY
#define LIBRAPID_ARRAY

#include <atomic>
#include <functional>
#include <librapid/array/extent.hpp>
#include <librapid/array/mapKernelUtils.hpp>
#include <librapid/array/multiarray_operations.hpp>
#include <librapid/array/ops.hpp>
#include <librapid/array/stride.hpp>
#include <librapid/autocast/autocast.hpp>
#include <librapid/config.hpp>
#include <librapid/math/rapid_math.hpp>
#include <ostream>
#include <variant>

namespace librapid {
	class AIterator;

	namespace utils {
		/**
		 * \rst
		 *
		 * Extract the dimensions of a nested list of vectors. For example,
		 * the following input would give the output specified below:
		 *
		 * .. code:: cpp
		 *
		 * 		{{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}, {{9, 10}, {11, 12}}}
		 * 		// Outputs {3, 2, 2}
		 *
		 * \endrst
		 */
		template<typename V>
		inline std::vector<int64_t> extractSize(const std::vector<V> &vec) {
			std::vector<int64_t> res(1);
			res[0] = vec.size();
			return res;
		}

		template<typename V>
		inline std::vector<int64_t>
		extractSize(const std::vector<std::vector<V>> &vec) {
			std::vector<int64_t> res(1);
			for (const auto &subVec : vec) {
				if (subVec.size() != vec[0].size()) {
					throw std::length_error(
					  "Not all vectors passed were the same length. Please"
					  " ensure that all sub-vectors have the same length");
				}
			}

			auto subSize = extractSize(vec[0]);
			res[0]		 = vec.size();
			res.insert(res.end(), subSize.begin(), subSize.end());
			return res;
		}
	} // namespace utils

	class Array;

	Array stack(const std::vector<Array>(&arrays), int64_t axis);

	class Array {
	public:
		/**
		 * \rst
		 *
		 * Default constructor for the Array type. It does not initialize any
		 * values and many functions will throw an error when given an empty
		 * array.
		 *
		 * \endrst
		 */
		Array();

		/**
		 * \rst
		 *
		 * Create a new Array from an Extent and an optional Datatype and
		 * Accelerator. The Extent defines the number of dimensions of the
		 * Array, as well as the size of each dimension.
		 *
		 * String values can also be passed as input parameters for the datatype
		 * and accelerator.
		 *
		 * The datatype can be constructed in many ways. For example, all of
		 * "i32", "int32" and "int" represent a 32-bit signed integer.
		 *
		 * All possible names are listed below. A green label means that name is
		 * safe to use. Yellow represents a value which should be avoided if
		 * possible, and red means the value is strongly advised not to be used.
		 *
		 * Please also note that the None type array may be very buggy and
		 * should not be used if possible.
		 *
		 * *Note: These warnings are more for readability than anything else*
		 *
		 * .. panels::
		 *		:container: container pb-4
		 *		:column: col-lg-6 col-md-6 col-sm-6 col-xs-12 p-2
		 *
		 *		None Type
		 *
		 *		:badge:`n, badge-danger`
		 *		:badge:`none, badge-success`
		 *		:badge:`null, badge-warning`
		 *		:badge:`void, badge-warning`
		 *
		 * 		---
		 *
		 *		Signed 32-bit integer
		 *
		 *		:badge:`i32, badge-success`
		 *		:badge:`int32, badge-success`
		 *		:badge:`long, badge-warning`
		 *
		 *		---
		 *
		 *		Signed 64-bit integer
		 *
		 *		:badge:`i, badge-danger`
		 *		:badge:`int, badge-warning`
		 *		:badge:`i64, badge-success`
		 *		:badge:`int64, badge-success`
		 *		:badge:`long long, badge-warning`
		 *
		 * 		---
		 *
		 *		32-bit floating point
		 *
		 *		:badge:`f32, badge-success`
		 *		:badge:`float32, badge-success`
		 *		:badge:`float, badge-warning`
		 *
		 * 		---
		 *
		 *		64-bit floating point
		 *
		 *		:badge:`f, badge-danger`
		 *		:badge:`f64, badge-success`
		 *		:badge:`float64, badge-success`
		 *		:badge:`double, badge-warning`
		 *
		 *		---
		 *
		 *		32-bit complex number
		 *
		 * 		:badge:`cf32, badge-success`
		 * 		:badge:`cfloat32, badge-success`
		 * 		:badge:`complex float, badge-success`
		 *
		 *		---
		 *
		 *		64-bit complex number
		 *
		 * 		:badge:`c, badge-danger`
		 * 		:badge:`cf, badge-danger`
		 * 		:badge:`cf64, badge-success`
		 * 		:badge:`cfloat64, badge-success`
		 * 		:badge:`complex, badge-warning`
		 * 		:badge:`complex double, badge-success`
		 *
		 *
		 * The accelerator value must be "CPU" or "GPU".
		 *
		 * Parameters
		 * ----------
		 *
		 * extent: ``Extent``
		 *		The dimensions for the Array
		 * dtype: ``Datatype = FLOAT64``
		 *		The datatype for the Array
		 * location: ``Accelerator = CPU``
		 *		Where the Array will be stored. GPU is only allowed if CUDA
		 *support is enabled at compile time
		 *
		 * \endrst
		 */
		Array(const Extent &extent, Datatype dtype = Datatype::FLOAT64,
			  Accelerator location = Accelerator::CPU);

#ifndef LIBRAPID_DOXYGEN_BUILD
		inline Array(const Extent &extent, const std::string &dtype,
					 Accelerator location = Accelerator::CPU) :
				Array(extent, stringToDatatype(dtype), location) {}

		inline Array(const Extent &extent, Datatype dtype,
					 const std::string &accelerator = "cpu") :
				Array(extent, dtype, stringToAccelerator(accelerator)) {}

		inline Array(const Extent &extent, const std::string &dtype,
					 const std::string &accelerator) :
				Array(extent, stringToDatatype(dtype),
					  stringToAccelerator(accelerator)) {}
#endif // LIBRAPID_DOXYGEN_BUILD

		/**
		 * \rst
		 *
		 * Create an Array object from an existing one. This constructor copies
		 * all values, and the new Array shares the data of the Array passed to
		 * it, so an update in one will result in an update in the other.
		 *
		 * Note that if the input Array is not initialized, the function will
		 * quick return and not initialize the new Array.
		 *
		 * Arguments can be passed to `dtype` and `locn` to cast the type of the
		 * input array.
		 *
		 * .. Attention::
		 *		If arguments are passed to `dtype` or `location`, the array
		 * 		might end up being copied, rather than referenced. Please be
		 * 		aware of this when making use of this functionality.
		 *
		 * .. Hint::
		 *		If you want to create an exact copy of an Array, but don't want
		 *		the data to be linked, see the ``Array::copy()`` function.
		 *
		 * Parameters
		 * ----------
		 *
		 * other: ``Array``
		 *		The Array instance to construct from
		 * dtype: ``Datatype``
		 *		The datatype to construct with (defaults to input datatype)
		 * locn: ``Accelerator``
		 *		The location to construct with (defaults to input location)
		 *
		 * \endrst
		 */
		Array(const Array &other, Datatype dtype = Datatype::NONE,
			  Accelerator locn = Accelerator::NONE);

		/**
		 * \rst
		 *
		 * Create an array from a scalar value. The array will be created on
		 * host memory (even if CUDA is enabled) and will be stored as a
		 * zero-dimensional Array.
		 *
		 * \endrst
		 */
		Array(bool val, Datatype dtype = Datatype::INT64,
			  Accelerator locn = Accelerator::CPU);

#ifndef LIBRAPID_DOXYGEN_BUILD
		Array(float val, Datatype dtype = Datatype::FLOAT32,
			  Accelerator locn = Accelerator::CPU);

		Array(double val, Datatype dtype = Datatype::FLOAT64,
			  Accelerator locn = Accelerator::CPU);

		inline Array(bool val, const std::string &dtype,
					 Accelerator locn = Accelerator::CPU) :
				Array(val, stringToDatatype(dtype), locn) {}

		inline Array(float val, const std::string &dtype,
					 Accelerator locn = Accelerator::CPU) :
				Array(val, stringToDatatype(dtype), locn) {}

		inline Array(double val, const std::string &dtype,
					 Accelerator locn = Accelerator::CPU) :
				Array(val, stringToDatatype(dtype), locn) {}

		inline Array(bool val, Datatype dtype, const std::string &locn) :
				Array(val, dtype, stringToAccelerator(locn)) {}

		inline Array(float val, Datatype dtype, const std::string &locn) :
				Array(val, dtype, stringToAccelerator(locn)) {}

		inline Array(double val, Datatype dtype, const std::string &locn) :
				Array(val, dtype, stringToAccelerator(locn)) {}

		inline Array(bool val, const std::string &dtype,
					 const std::string &locn) :
				Array(val, stringToDatatype(locn), stringToAccelerator(locn)) {}

		inline Array(float val, const std::string &dtype,
					 const std::string &locn) :
				Array(val, stringToDatatype(locn), stringToAccelerator(locn)) {}

		inline Array(double val, const std::string &dtype,
					 const std::string &locn) :
				Array(val, stringToDatatype(locn), stringToAccelerator(locn)) {}

		template<typename T, typename std::enable_if<std::is_integral<T>::value,
													 int>::type = 0>
		inline Array(T val, Datatype dtype = Datatype::INT64,
					 Accelerator locn = Accelerator::CPU) {
			initializeCudaStream();

			constructNew(Extent(1), Stride(1), dtype, locn);
			m_isScalar = true;
			if (locn == Accelerator::CPU) {
				std::visit([&](auto *data) { *data = val; }, m_dataStart);
			}
#	ifdef LIBRAPID_HAS_CUDA
			else {
				int64_t tempVal	 = val;
				RawArray tempDst = RawArray {m_dataStart, dtype, locn};
				RawArray tempSrc = RawArray {
				  RawArrayData(&tempVal), Datatype::INT64, Accelerator::CPU};
				rawArrayMemcpy(tempDst, tempSrc, 1);
			}
#	else
			else {
				throw std::invalid_argument(
				  "CUDA support was not enabled, "
				  "so a value cannot be created on the GPU");
			}
#	endif // LIBRAPID_HAS_CUDA
		}

		template<typename T, typename std::enable_if<std::is_integral<T>::value,
													 int>::type = 0>
		inline Array(T val, const std::string &dtype,
					 Accelerator locn = Accelerator::CPU) :
				Array(val, stringToDatatype(dtype), locn) {}

		template<typename T, typename std::enable_if<std::is_integral<T>::value,
													 int>::type = 0>
		inline Array(T val, Datatype dtype, const std::string &locn) :
				Array(val, dtype, stringToAccelerator(locn)) {}

		template<typename T, typename std::enable_if<std::is_integral<T>::value,
													 int>::type = 0>
		inline Array(T val, const std::string &dtype, const std::string &locn) :
				Array(val, stringToDatatype(dtype), stringToAccelerator(locn)) {
		}

#endif // LIBRAPID_DOXYGEN_BUILD

		// TODO: Make this do something useful
		// inline Array(Array &&other) = default;

		// Only show the data-constructors if we're compiling the library. They
		// are just annoying in the documentation
#ifndef LIBRAPID_DOXYGEN_BUILD
#	pragma region ENUM_CONSTRUCTORS
#	define CONSTRUCTOR_TEMPLATE                                               \
		template<                                                              \
		  typename V,                                                          \
		  typename std::enable_if<std::is_scalar<V>::value, int>::type = 0>

#	define CONSTRUCTOR_BODY_INIT(TYPE)                                        \
		Array(const TYPE &values,                                              \
			  Datatype dtype   = Datatype::NONE,                               \
			  Accelerator locn = Accelerator::CPU) {                           \
			std::vector<Array> toStack;                                        \
			for (const auto &sub : values)                                     \
				toStack.emplace_back(                                          \
				  Array(sub,                                                   \
						dtype == Datatype::NONE ? typeToDatatype<V>() : dtype, \
						locn));                                                \
			auto stacked = librapid::stack(toStack, 0);                        \
			*this		 = (stacked);                                          \
		}

#	define CVEC  std::vector
#	define CINIT std::initializer_list

		CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<V>) // 1D

		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CINIT<V>)

		  // 2D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<CVEC<V>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CINIT<CINIT<V>>)

		  // 3D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<V>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<V>>>)

		  // 4D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<V>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<CINIT<V>>>>)

		  // 5D
		  CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>)

		  // 6D
		  CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>)

		  // 7D
		  CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>)

		  // 8D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>>)

		  // 9D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>>>)

		  // 10D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(
			CINIT<
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>>>>)

#	undef CONSTRUCTOR_TEMPLATE
#	undef CONSTRUCTOR_BODY_INIT
#	undef CONSTRUCTOR_BODY_INIT
#	undef CVEC
#	undef CINIT
#	pragma endregion ENUM_CONSTRUCTORS

#	pragma region STRING_ENUM_CONSTRUCTORS
#	define CONSTRUCTOR_TEMPLATE                                               \
		template<                                                              \
		  typename V,                                                          \
		  typename std::enable_if<std::is_scalar<V>::value, int>::type = 0>

#	define CONSTRUCTOR_BODY_INIT(TYPE)                                        \
		Array(const TYPE &values,                                              \
			  const std::string &dtype,                                        \
			  Accelerator locn = Accelerator::CPU) {                           \
			std::vector<Array> toStack;                                        \
			for (const auto &sub : values)                                     \
				toStack.emplace_back(Array(sub,                                \
										   dtype.empty()                       \
											 ? typeToDatatype<V>()             \
											 : stringToDatatype(dtype),        \
										   locn));                             \
			auto stacked = librapid::stack(toStack, 0);                        \
			*this		 = (stacked);                                          \
		}

#	define CVEC  std::vector
#	define CINIT std::initializer_list

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<V>) // 1D

		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CINIT<V>)

		  // 2D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<CVEC<V>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CINIT<CINIT<V>>)

		  // 3D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<V>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<V>>>)

		  // 4D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<V>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<CINIT<V>>>>)

		  // 5D
		  CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>)

		  // 6D
		  CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>)

		  // 7D
		  CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>)

		  // 8D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>>)

		  // 9D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>>>)

		  // 10D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(
			CINIT<
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>>>>)

#	undef CONSTRUCTOR_TEMPLATE
#	undef CONSTRUCTOR_BODY_INIT
#	undef CONSTRUCTOR_BODY_INIT
#	undef CVEC
#	undef CINIT
#	pragma endregion STRING_ENUM_CONSTRUCTORS

#	pragma region ENUM_STRING_CONSTUCTORS
#	define CONSTRUCTOR_TEMPLATE                                               \
		template<                                                              \
		  typename V,                                                          \
		  typename std::enable_if<std::is_scalar<V>::value, int>::type = 0>

#	define CONSTRUCTOR_BODY_INIT(TYPE)                                        \
		Array(const TYPE &values, Datatype dtype, const std::string &locn) {   \
			std::vector<Array> toStack;                                        \
			for (const auto &sub : values)                                     \
				toStack.emplace_back(                                          \
				  Array(sub,                                                   \
						dtype == Datatype::NONE ? typeToDatatype<V>() : dtype, \
						locn.empty() ? Accelerator::CPU                        \
									 : stringToAccelerator(locn)));            \
			auto stacked = librapid::stack(toStack, 0);                        \
			*this		 = (stacked);                                          \
		}

#	define CVEC  std::vector
#	define CINIT std::initializer_list

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<V>) // 1D

		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CINIT<V>)

		  // 2D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<CVEC<V>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CINIT<CINIT<V>>)

		  // 3D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<V>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<V>>>)

		  // 4D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<V>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<CINIT<V>>>>)

		  // 5D
		  CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>)

		  // 6D
		  CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>)

		  // 7D
		  CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>)

		  // 8D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>>)

		  // 9D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>>>)

		  // 10D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(
			CINIT<
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>>>>)

#	undef CONSTRUCTOR_TEMPLATE
#	undef CONSTRUCTOR_BODY_INIT
#	undef CONSTRUCTOR_BODY_INIT
#	undef CVEC
#	undef CINIT
#	pragma endregion ENUM_STRING_CONSTUCTORS

#	pragma region STRING_STRING_CONSTRUCTORS
#	define CONSTRUCTOR_TEMPLATE                                               \
		template<                                                              \
		  typename V,                                                          \
		  typename std::enable_if<std::is_scalar<V>::value, int>::type = 0>

#	define CONSTRUCTOR_BODY_INIT(TYPE)                                        \
		Array(const TYPE &values,                                              \
			  const std::string &dtype,                                        \
			  const std::string &locn) {                                       \
			std::vector<Array> toStack;                                        \
			for (const auto &sub : values)                                     \
				toStack.emplace_back(                                          \
				  Array(sub,                                                   \
						dtype.empty() ? datatypeToString(typeToDatatype<V>())  \
									  : dtype,                                 \
						locn.empty() ? Accelerator::CPU                        \
									 : stringToAccelerator(locn)));            \
			auto stacked = librapid::stack(toStack, 0);                        \
			*this		 = (stacked);                                          \
		}

#	define CVEC  std::vector
#	define CINIT std::initializer_list

		  // 1D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<V>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CINIT<V>)

		  // 2D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<CVEC<V>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CINIT<CINIT<V>>)

		  // 3D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<V>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<V>>>)

		  // 4D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<V>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<CINIT<V>>>>)

		  // 5D
		  CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>)

		  // 6D
		  CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>)

		  // 7D
		  CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>)

		  // 8D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>>)

		  // 9D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>>>)

			CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>>>)

		  // 10D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(
			CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<CVEC<V>>>>>>>>>>)

			CONSTRUCTOR_TEMPLATE
		  CONSTRUCTOR_BODY_INIT(
			CINIT<
			  CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<CINIT<V>>>>>>>>>>)

#	undef CONSTRUCTOR_TEMPLATE
#	undef CONSTRUCTOR_BODY_INIT
#	undef CONSTRUCTOR_BODY_INIT
#	undef CVEC
#	undef CINIT
#	pragma endregion STRING_STRING_CONSTRUCTORS
#else
		// Show a single from-data constructor for the documentation

#	define CONSTRUCTOR_TEMPLATE                                               \
		template<                                                              \
		  typename V,                                                          \
		  typename std::enable_if<std::is_scalar<V>::value, int>::type = 0>

#	define CONSTRUCTOR_BODY_INIT(TYPE)                                        \
		Array(const TYPE &values,                                              \
			  Datatype dtype   = Datatype::NONE,                               \
			  Accelerator locn = Accelerator::CPU) {                           \
			std::vector<Array> toStack;                                        \
			for (const auto &sub : values)                                     \
				toStack.emplace_back(                                          \
				  Array(sub,                                                   \
						dtype == Datatype::NONE ? typeToDatatype<V>() : dtype, \
						locn));                                                \
			auto stacked = librapid::stack(toStack, 0);                        \
			*this		 = (stacked);                                          \
		}

#	define CVEC  std::vector
#	define CINIT std::initializer_list

		CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CVEC<V>)	   // 1D
		  CONSTRUCTOR_TEMPLATE CONSTRUCTOR_BODY_INIT(CINIT<V>) // 1D

#	undef CONSTRUCTOR_TEMPLATE
#	undef CONSTRUCTOR_BODY_INIT
#	undef CVEC
#	undef CINIT
#endif // LIBRAPID_DOXYGEN_BUILD

		  /**
		   * \rst
		   *
		   * Set one Array equal to another Array or to a scalar.
		   *
		   * This function will link two arrays together. This means that
		   * changing a value in one array might end up changing an array in
		   * another. The arrays are not required to have the same extent
		   * (dimensions), stride, datatype or accelerator (location).
		   *
		   * .. code-block::
		   * 	:caption: A C++ Example
		   *
		   * 	... // Set everything up
		   *
		   * 	namespace lrc = librapid;
		   *
		   * 	lrc::Array myFirstArray({1, 2, 3, 4, 5});
		   * 	lrc::Array mySecondArray({10, 20, 30});
		   *
		   * 	mySecondArray = myFirstArray; // Copies data by reference
		   *
		   * 	std::cout << mySecondArray << "\n"; // [1 2 3 4 5]
		   *
		   * 	mySecondArray[0] = 123; // Updates myFirstArray too :)
		   *
		   * 	std::cout << myFirstArray << "\n"; // [123 2 3 4 5]
		   *
		   * 	... // Clean everything up
		   *
		   * .. Attention::
		   *		There is a single exception to this, which occurs when
		   * 		this array is a direct subscript of another (e.g.
		   * 		``myArray[0]``). If this is the case, the left-hand-side
		   * 		of this operation *must* have the same extent, otherwise
		   * 		an error will be thrown. Additionally, data will be COPIED
		   * 		instead of references being made. This means updates will
		   * 		not carry across multiple Arrays.
		   *
		   * .. Attention::
		   *		Be VERY careful when using this feature, as it can lead to
		   *		unwanted side effects. Take a look at ``lrc::Array::copy()``
		   *		for a function which will make an exact clone of an Array
		   *		without making any new references.
		   *
		   * Parameters
		   * ----------
		   * other: Basically anything
		   * 		The value to set the array to
		   *
		   * \endrst
		   */
		  Array &
		  operator=(const Array &other);

		Array &operator=(bool val);

		template<typename T, typename std::enable_if<std::is_integral<T>::value,
													 int>::type = 0>
		inline Array &operator=(T val) {
			if (m_isChild && !m_isScalar) {
				throw std::invalid_argument(
				  "Cannot set an array with more than zero"
				  " dimensions to a scalar value. Array must"
				  " have zero dimensions (i.e. scalar)");
			}
			if (!m_isChild) {
				if (m_references != nullptr) { decrement(); }
				constructNew(
				  Extent(1), Stride(1), Datatype::INT64, Accelerator::CPU);
			}

			auto raw	= createRaw();
			int64_t tmp = val;
			rawArrayMemcpy(
			  raw, RawArray {&tmp, Datatype::INT64, Accelerator::CPU}, 1);

			m_isScalar = true;
			return *this;
		}

		Array &operator=(float val);

		Array &operator=(double val);

		Array &operator=(Complex<double> val);

		/**
		 * \rst
		 *
		 * Set this array to another array. The only use I can think of for this
		 * is for iterators in Python, where changing the iterated variable
		 * doesn't actually update the value in the Array being iterated over.
		 * It quite literally just wrapps the C++ ``operator=()`` function...
		 *
		 * Examples
		 * --------
		 *
		 * .. code-block:: python
		 * 		:caption: This code works as expected
		 *
		 * 		import librapid as lrp
		 *
		 * 		myArray = lrp.Array([[1, 2, 3],
		 * 		                     [4, 5, 6]])
		 *
		 * 		for val in myArray:
		 * 			val[1] = 12345
		 *
		 * 		print(myArray)
		 * 		"""
		 * 		Outputs:
		 * 		[[    1 12345     3]
		 *       [    4 12345     6]]
		 * 		"""
		 *
		 * .. code-block:: python
		 * 		:caption: This code DOES NOT work as expected
		 *
		 * 		import librapid as lrp
		 *
		 * 		myArray = lrp.Array([[1, 2, 3],
		 * 		                     [4, 5, 6]])
		 *
		 * 		# Notice we are directly setting $val
		 * 		for val in myArray:
		 * 			val = lrp.Array([10, 20, 30])
		 *
		 * 			# Instead, you should do something like:
		 * 			# val.set(lrp.Array([10, 20, 30]))
		 *
		 * 		print(myArray)
		 * 		"""
		 * 		Outputs:
		 * 		[[1 2 3]
		 *       [4 5 6]]
		 * 		"""
		 *
		 * \endrst
		 */
		inline void set(const Array &other) { *this = other; }

		template<typename T>
		inline void set(const T &other) {
			*this = other;
		}

		~Array();

		/**
		 * \rst
		 *
		 * Returns true if and only if the two arrays are actually the same
		 * thing. For example, you could have two arrays x and y with the same
		 * stride, extent and data, but if the arrays don't share the same data
		 * (in memory), this function will return false.
		 *
		 * \endrst
		 */
		[[nodiscard]] bool isSame(const Array &other) const {
			return m_dataStart == other.m_dataStart &&
				   m_extent == other.m_extent && m_stride == other.m_stride &&
				   m_isScalar == other.m_isScalar;
		}

		/**
		 * \rst
		 *
		 * Return the number of dimensions of the Array
		 *
		 * \endrst
		 */
		[[nodiscard]] inline int64_t ndim() const { return m_extent.ndim(); }

		/**
		 * \rst
		 *
		 * Return the shape of the Array as an ``Extent`` object
		 *
		 * \endrst
		 */
		[[nodiscard]] inline Extent extent() const { return m_extent; }

		/**
		 * \rst
		 *
		 * Return the strides of the Array as a ``Stride`` object
		 *
		 * \endrst
		 */
		[[nodiscard]] inline Stride stride() const { return m_stride; }

		/**
		 * \rst
		 *
		 * Return true if this Array object is zero-dimensional (i.e. a scalar
		 * value)
		 *
		 * For example:
		 *
		 * .. code:: cpp
		 *
		 * 		auto x = librapid::Array(librapid::Extent({2, 3, 4}));
		 * 		x.isScalar(); // false -- array is multi-dimensional
		 * 		x[0].isScalar(); // false -- this is a matrix
		 * 		x[0][0].isScalar(); // false -- this is a vector
		 * 		x[0][0][0].isScalar(); // true -- this is a single value
		 *
		 * \endrst
		 */
		[[nodiscard]] inline bool isScalar() const { return m_isScalar; }

		/**
		 * \rst
		 *
		 * For C++ use only -- returns a VoidPtr object containing the memory
		 * location of the Array's data, its datatype and its location. This
		 * is intended mainly for internal usage, but may be useful for
		 * lower-level access to the underlying data of the Array.
		 *
		 * \endrst
		 */
		[[nodiscard]] RawArray createRaw() const;

		/**
		 * \rst
		 *
		 * Return the datatype of the Array (``INT64``, ``FLOAT32``,
		 * ``FLOAT64``, etc.)
		 *
		 * \endrst
		 */
		[[nodiscard]] inline Datatype dtype() const { return m_dtype; }

		/**
		 * \rst
		 *
		 * Return the accelerator of the Array (``CPU``/``GPU``)
		 *
		 * \endrst
		 */
		[[nodiscard]] inline Accelerator location() const { return m_location; }

		/**
		 * \rst
		 *
		 * Return the size of the first axis of the array. This can be used
		 * for iterating over an array, for example.
		 *
		 * This is equivalent to ``myArray.extent()[0]``
		 *
		 * \endrst
		 */
		[[nodiscard]] inline int64_t len() const { return m_extent[0]; }

		/**
		 * \rst
		 *
		 * Cast a scalar array into a specific type.
		 *
		 * If this array is not a scalar, this function will throw an exception
		 *
		 * \endrst
		 */
		template<typename T>
		[[nodiscard]] inline operator T() const {
			if (!m_isScalar) {
				throw std::invalid_argument("Cannot convert Array with " +
											m_extent.str() +
											" to scalar value");
			}

			if (m_location == Accelerator::CPU) {
				switch (m_dtype) {
					case Datatype::NONE:
					case Datatype::VALIDNONE:
						throw std::invalid_argument(
						  "Cannot convert uninitialized array to scalar value");
					case Datatype::INT32:
						return (T)*std::get<int32_t *>(m_dataStart);
					case Datatype::INT64:
						return (T)*std::get<int64_t *>(m_dataStart);
					case Datatype::FLOAT32:
						return (T)*std::get<float *>(m_dataStart);
					case Datatype::FLOAT64:
						return (T)*std::get<double *>(m_dataStart);
					case Datatype::CFLOAT32:
						return (T)*std::get<Complex<float> *>(m_dataStart);
					case Datatype::CFLOAT64:
						return (T)*std::get<Complex<double> *>(m_dataStart);
				}
			} else if (m_location == Accelerator::GPU) {
				Datatype resType = typeToDatatype<T>();
				RawArrayData raw;
				auto tmp = RawArray {raw, resType, Accelerator::CPU};
				rawArrayMalloc(tmp, 1);
				rawArrayMemcpy(tmp, createRaw(), 1);
				T res;

				switch (resType) {
					case Datatype::NONE:
					case Datatype::VALIDNONE: {
						rawArrayFree(tmp);
						throw std::invalid_argument(
						  "Cannot convert uninitialized array to scalar value");
					}
					case Datatype::INT32: {
						res = (T)*std::get<int32_t *>(tmp.data);
						break;
					}
					case Datatype::INT64: {
						res = (T)*std::get<int64_t *>(tmp.data);
						break;
					}
					case Datatype::FLOAT32: {
						res = (T)*std::get<float *>(tmp.data);
						break;
					}
					case Datatype::FLOAT64: {
						res = (T)*std::get<double *>(tmp.data);
						break;
					}
					case Datatype::CFLOAT32: {
						res = (T)*std::get<Complex<float> *>(tmp.data);
						break;
					}
					case Datatype::CFLOAT64: {
						res = (T)*std::get<Complex<double> *>(tmp.data);
						break;
					}
				}

				rawArrayFree(tmp);
				return res;
			}

			throw std::runtime_error("Invalid accelerator used in Array cast");
		}

		/**
		 * \rst
		 *
		 * Return a sub-array or scalar value at a particular index in the
		 * Array.
		 *
		 * .. Hint::
		 * 		The ``subscript`` function is overloaded as ``[<index>]`` in
		 * 		both C++ and Python, and it is strongly advised to use normal
		 * 		indexing over this function to improve readability.
		 *
		 * .. Attention::
		 * 		The index must be in the range ``[0, array.len())``,
		 * 		otherwise an exception will be thrown. This may be changed in
		 * 		the future to support negative indices, but there are
		 * 		currently no plans to do so.
		 *
		 * Examples
		 * --------
		 *
		 *
		 * .. code-block:: python
		 * 		:caption: A Python Example
		 *
		 * 		import librapid as lrp
		 *
		 * 		x = lrp.Array([[1, 2, 3],
		 * 		               [4, 5, 6]])
		 *
		 * 		#########################
		 * 		# Accessing a sub-array #
		 * 		#########################
		 * 		print(x.subscript(0)) # Outputs [1, 2, 3]
		 * 		print(x[0]) # Also outputs [1, 2, 3]
		 *
		 * 		print(x.subscript(1)) # Outputs [4, 5, 6]
		 * 		print(x[1]) # Outputs [4, 5, 6]
		 *
		 * 		############################
		 * 		# Accessing a single value #
		 * 		############################
		 * 		print(x.subscript(0).subscript(0)) # Outputs 1
		 * 		print(x[0][0]) # Also outputs 1 -- much neater code though :)
		 *
		 * 		print(x[1][2]) # Outputs 6
		 *
		 *
		 * .. code-block:: cpp
		 *  	:caption: A C++ Example
		 *
		 * 		#include <librapid/librapid.hpp>
		 *
		 * 		using namespace lrc = librapid;
		 *
		 * 		// ............
		 *
		 * 		auto x = lrc::Array({{1, 2, 3},
		 * 		                     {4, 5, 6}});
		 *
		 * 		//=======================//
		 * 		// Accessing a sub-array //
		 * 		//=======================//
		 * 		fmt::print("{}\n", x.subscript(0)); // Outputs [1, 2, 3]
		 * 		fmt::print("{}\n", x[0]; // Also outputs [1, 2, 3]
		 *
		 * 		fmt::print("{}\n", x.subscript(1)); // Outputs [4, 5, 6]
		 * 		fmt::print("{}\n", x[1]; // Also outputs [4, 5, 6]
		 *
		 * 		//==========================//
		 * 		// Accessing a single value //
		 * 		//==========================//
		 * 		fmt::print("{}\n", x.subscript(0).subscript(0)); // Outputs 1
		 * 		fmt::print("{}\n", x[0][0]); // Also outputs 1 -- neater code
		 *
		 * 		fmt::print("{}\n", x.subscript(1).subscript(2)); // Outputs 6
		 * 		fmt::print("{}\n", x[1][2]); // Also outputs 6
		 *
		 * 		// ............
		 *
		 * \endrst
		 */
		[[nodiscard]] Array subscript(int64_t index) const;

		/**
		 * \rst
		 *
		 * Please see:
		 * .. doxygenfunction:: librapid::Array::subscript(int64_t) const
		 *
		 * \endrst
		 */
		inline const Array operator[](int64_t index) const {
			return subscript(index);
		}

		/**
		 * \rst
		 *
		 * Please see:
		 * .. doxygenfunction:: librapid::Array::subscript(int64_t) const
		 *
		 * \endrst
		 */
		inline Array operator[](int64_t index) {
			using nonConst = typename std::remove_const<Array>::type;
			return static_cast<nonConst>(subscript(index));
		}

		/**
		 * \rst
		 *
		 * Create an exact copy of an array and it's data and return the result.
		 *
		 * .. Hint::
		 *
		 *		When cloning an array, the data is optimized in memory
		 *		to improve performance. Furthermore, and transformations
		 *		applied to the parent matrix will be optimized out and
		 *		calculated fully, further improving performance
		 *
		 * \endrst
		 */
		[[nodiscard]] Array clone(Datatype dtype   = Datatype::NONE,
								  Accelerator locn = Accelerator::NONE) const;

		// Don't show these in the documentation if we don't need to
#ifndef LIBRAPID_DOXYGEN_BUILD
		[[nodiscard]] Array clone(const std::string &dtype,
								  Accelerator locn = Accelerator::NONE) const;

		[[nodiscard]] Array clone(Datatype dtype,
								  const std::string &locn = "none") const;

		[[nodiscard]] Array clone(const std::string &dtype,
								  const std::string &locn) const;
#endif // LIBRAPID_DOXYGEN_BUILD

		/**
		 * \rst
		 *
		 * Fill the every element of the Array with a particular value
		 *
		 * \endrst
		 */
		void fill(double val);

		/**
		 * \rst
		 *
		 * Return a copy of this array filled with a particular value
		 *
		 * \endrst
		 */
		[[nodiscard]] inline Array filled(double val) {
			auto res = Array(m_extent, m_dtype, m_location);
			res.fill(val);
			return res;
		}

		/**
		 * \rst
		 *
		 * Fill an array with uniformly random numbers ranging from ``min``
		 * to ``max``. You can also set the ``seed`` parameter to get
		 * predictable outcomes.
		 *
		 * .. Hint::
		 * 		For floating point arrays, the range of values that can be
		 * 		taken range from :math:`[min, max)`, while for integer
		 * 		arrays, the output range is from :math:`[min, max]`
		 *
		 * Parameters
		 * ----------
		 * min: Scalar value (integer, floating point, etc.)
		 * 		The minimum value for the random value
		 * max: Scalar value
		 * 		The maximum value for the random value
		 * seed: uint64_t
		 * 		The seed for the PRNG
		 *
		 * \endrst
		 */
		template<typename T = double>
		inline void fillRandom(T min = 0, T max = 1, uint64_t seed = -1) {
			static uint64_t statSeed = 0;
			static uint64_t prevSeed = -1;
			static bool statSeedSet	 = false;

			if (prevSeed != seed || !statSeedSet || seed != (uint64_t)-1) {
				prevSeed	= seed;
				statSeed	= seed == -1 ? (uint64_t)(seconds() * 10) : seed;
				statSeedSet = true;
			}

			applyUnaryOp(*this, *this, ops::FillRandom<T>(min, max, statSeed));
		}

		template<typename T = double>
		inline Array filledRandom(T min = 0, T max = 1,
								  int64_t seed = -1) const {
			auto res = Array(m_extent, m_dtype, m_location);
			res.fillRandom(min, max, seed);
			return res;
		}

		/**
		 * \rst
		 *
		 * Adjust the extent (shape) of an array and return the result.
		 *
		 * In order to reshape the array, the new number of elements **MUST** be
		 * the same as the number of elements in the old array, otherwise an
		 *error will be thrown.
		 *
		 * It is also possible to use an automatic dimension by passing in
		 * ``librapid::AUTO`` (equals `-1`) to the shape. This will be evaulated
		 * when the function is called. If the array cannot be reshaped into
		 * the new shape, even with an automatic dimension, an error will be
		 *thrown
		 *
		 * Examples
		 * --------
		 *
		 * .. code-block:: python
		 *		:caption: Example written in Python
		 *
		 *		my_matrix = librapid.ndarray([[1, 2, 3], [4, 5, 6]])
		 *		print(my_matrix)
		 *		# [[1 2 3]
		 *		#  [4 5 6]]
		 *
		 *		my_matrix.reshape([6])
		 *
		 *		print(my_matrix)
		 *		# [1 2 3 4 5 6]
		 *
		 * .. code-block:: python
		 *		:caption: Example written in Python
		 *
		 *		my_matrix = librapid.ndarray([[1, 2, 3, 4], [5, 6, 7, 8]])
		 *		print(my_matrix)
		 *		# [[1. 2. 3. 4.]
		 *		#  [5. 6. 7. 8.]]
		 *
		 *		my_matrix.reshape([2, 2, librapid.AUTO])
		 *
		 *		print(my_matrix)
		 *		# [[[1. 2.]
		 *		#   [3. 4.]]
		 *		#
		 *		#  [[5. 6.]
		 *		#   [7. 8.]]]
		 *
		 * When possible, the resulting array references the same data
		 * as the input array, however this is not possible if the array
		 * has a non-trivial stride (for example, if the array has been
		 * previously transposed)
		 *
		 * Parameters
		 * ----------
		 *
		 * new_shape: extent, vector, initializer_list, list, tuple, *args
		 *		The new shape for the array
		 *
		 * \endrst
		 */
		void reshape(const Extent &newShape);

		inline void reshape(const std::vector<int64_t> &newShape) {
			reshape(Extent(newShape));
		}

		[[nodiscard]] inline Array reshaped(const Extent &newShape) const {
			Array res = clone();
			res.reshape(newShape);
			return res;
		}

		[[nodiscard]] inline Array
		reshaped(const std::vector<int64_t> &newShape) const {
			return reshaped(Extent(newShape));
		}

		[[nodiscard]] Array copy(const Datatype &dtype	 = Datatype::NONE,
								 const Accelerator &locn = Accelerator::NONE);

		template<typename PTR, typename First>
		static inline void
		mapKernelGetPointers(PTR **__restrict ptrVals, const Extent &e,
							 const Datatype &d, First first) {
			*ptrVals = (PTR *)extractVoidPtr(first.createRaw());
		}

		template<typename PTR, typename First, typename... Pack>
		static inline void mapKernelGetPointers(PTR **__restrict ptrVals,
												const Extent &e,
												const Datatype &d, First first,
												Pack... arrayPack) {
			// Ensure all the arrays are on the CPU, have the same stride, are
			// contiguous in memory, have the same extent and the same datatype
			if (typeToDatatype<PTR>() != first.m_dtype ||
				!first.m_stride.isContiguous() || first.m_extent != e ||
				first.m_dtype != d) {
				throw std::runtime_error(
				  "Please read the documentation for valid inputs to the "
				  "mapKernel function");
			}

			*ptrVals = (PTR *)extractVoidPtr(first.createRaw());
			mapKernelGetPointers(ptrVals + 1, e, d, arrayPack...);
		}

		template<typename Kernel>
		inline void map(const Kernel &kernel) {
			applyUnaryOp(*this, *this, kernel);
		}

		template<typename Kernel>
		inline Array mapped(const Kernel &kernel) const {
			Array res(m_extent, m_dtype, m_location);
			applyUnaryOp(res, *this, kernel);
			return res;
		}

		template<typename Kernel, typename... Pack>
		static inline Array &mapKernel(const Kernel &kernel,
									   Pack... arrayPack) {
			auto arrays =
			  std::forward_as_tuple(std::forward<Pack>(arrayPack)...);
			// To get an array: std::get<i>(arrays);

			Array &dst = std::get<sizeof...(Pack) - 1>(arrays);
			// For now, assume every array is trivial and on the CPU

			std::visit(
			  [&](auto *dstPtr) {
				  using TYPE =
					typename std::remove_pointer<decltype(dstPtr)>::type;
				  TYPE *pointers[sizeof...(Pack)];
				  mapKernelGetPointers(
					&(pointers[0]), dst.extent(), dst.dtype(), arrayPack...);

				  int64_t end = dst.extent().size();

				  if (dst.location() == Accelerator::CPU) {
#ifdef LIBRAPID_PYTHON
					  for (int64_t i = 0; i < end; ++i) {
						  utils::ApplyKernelImpl<TYPE,
												 Kernel,
												 sizeof...(Pack) -
												   1>::run((TYPE **)pointers,
														   dstPtr,
														   kernel,
														   i);
					  }
#else // LIBRAPID_PYTHON
					  if (end < 2500) {
						  for (int64_t i = 0; i < end; ++i) {
							  utils::ApplyKernelImpl<
								TYPE,
								Kernel,
								sizeof...(Pack) - 1>::run((TYPE **)pointers,
														  dstPtr,
														  kernel,
														  i);
						  }
					  } else {
						  auto localKernel = kernel;
#	pragma omp parallel for shared(                                           \
	  pointers, dstPtr, localKernel, end) default(none)
						  for (int64_t i = 0; i < end; ++i) {
							  utils::ApplyKernelImpl<
								TYPE,
								Kernel,
								sizeof...(Pack) - 1>::run((TYPE **)pointers,
														  dstPtr,
														  localKernel,
														  i);
						  }
					  }
#endif // LIBRAPID_PYTHON
#ifdef LIBRAPID_HAS_CUDA
				  } else {
					  // Copy the pointers to the GPU
					  static TYPE **gpuPointers =
						nullptr; // Static storage space for pointers
					  static uint64_t maxPointers = 16; // Size of "gpuPointers"

					  if (gpuPointers == nullptr) {
#	ifdef LIBRAPID_CUDA_STREAM
						  cudaSafeCall(
							cudaMallocAsync(&gpuPointers,
											sizeof(TYPE *) * maxPointers,
											cudaStream));
#	else
						  cudaSafeCall(cudaMalloc(
							&gpuPointers, sizeof(TYPE *) * maxPointers));
#	endif
					  }

					  if (sizeof...(Pack) - 1 > maxPointers) {
						  maxPointers <<= 1;

#	ifdef LIBRAPID_CUDA_STREAM
						  cudaSafeCall(cudaFreeAsync(gpuPointers, cudaStream));
#	else
						  cudaSafeCall(cudaFreeAsync(gpuPointers));
#	endif

#	ifdef LIBRAPID_CUDA_STREAM
						  cudaSafeCall(
							cudaMallocAsync(&gpuPointers,
											sizeof(TYPE *) * maxPointers,
											cudaStream));
#	else
						  cudaSafeCall(cudaMalloc(
							&gpuPointers, sizeof(TYPE *) * maxPointers));
#	endif
					  }

#	ifdef LIBRAPID_CUDA_STREAM
					  cudaSafeCall(
						cudaMemcpyAsync(gpuPointers,
										static_cast<TYPE **>(pointers),
										sizeof(TYPE *) * (sizeof...(Pack) - 1),
										cudaMemcpyHostToDevice,
										cudaStream));
#	else
					  cudaSafeCall(
						cudaMemcpy(gpuPointers,
								   static_cast<TYPE **>(pointers),
								   sizeof(TYPE *) * (sizeof...(Pack) - 1),
								   cudaMemcpyHostToDevice));
#	endif

					  using jitify::reflection::Type;

					  if constexpr (!utils::HasName<Kernel> {} ||
									!utils::HasKernel<Kernel> {}) {
						  throw std::runtime_error(
							"Invalid GPU Kernel! Must have name and kernel "
							"members");
						  return;
					  } else {
						  std::string args;
						  std::string indices;

						  switch (sizeof...(Pack) - 1) {
							  case 0: {
								  args	  = "";
								  indices = "";
								  break;
							  }
							  case 1: {
								  args	  = "T val0";
								  indices = "srcPointers[0][kernelIndex]";
								  break;
							  }
							  case 2: {
								  args =
									"T val0,"
									"T val1";
								  indices =
									"srcPointers[0][kernelIndex],"
									"srcPointers[1][kernelIndex]";
								  break;
							  }
							  case 3: {
								  args =
									"T val0,"
									"T val1,"
									"T val2";
								  indices =
									"srcPointers[0][kernelIndex],"
									"srcPointers[1][kernelIndex],"
									"srcPointers[2][kernelIndex]";
								  break;
							  }
							  case 4: {
								  args =
									"T val0,"
									"T val1,"
									"T val2,"
									"T val3";
								  indices =
									"srcPointers[0][kernelIndex],"
									"srcPointers[1][kernelIndex],"
									"srcPointers[2][kernelIndex],"
									"srcPointers[3][kernelIndex]";
								  break;
							  }
							  case 5: {
								  args =
									"T val0,"
									"T val1,"
									"T val2,"
									"T val3,"
									"T val4";
								  indices =
									"srcPointers[0][kernelIndex],"
									"srcPointers[1][kernelIndex],"
									"srcPointers[2][kernelIndex],"
									"srcPointers[3][kernelIndex],"
									"srcPointers[4][kernelIndex]";
								  break;
							  }
							  default: {
								  for (int64_t i = 0; i < sizeof...(Pack) - 1;
									   ++i) {
									  args += fmt::format("T val{}", i);
									  if (i + 2 < sizeof...(Pack)) {
										  args += ", ";
									  }
								  }

								  for (int64_t i = 0; i < sizeof...(Pack) - 1;
									   ++i) {
									  indices += fmt::format(
										"srcPointers[{}][kernelIndex]", i);
									  if (i + 2 < sizeof...(Pack)) {
										  indices += ", ";
									  }
								  }
								  break;
							  }
						  }

						  std::string gpuKernel = fmt::format(
							R"V0G0N(mapKernel
						__constant__ int LIBRAPID_MAX_DIMS = {0};
						#include <stdint.h>
						#include <type_traits>
						#include <{1}/curand_kernel.h>
						#include <{1}/curand.h>

						// Complex numbers :)
						{2}

						template<typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
						__global__
						inline T random(T lower, T upper, uint64_t seed = -1) {{
						// Random floating point value in range [lower, upper)

						static std::uniform_real_distribution<T> distribution(0., 1.);
						static std::mt19937 generator(seed == (uint64_t) -1 ? (unsigned int) (seconds() * 10) : seed);
						return lower + (upper - lower) * distribution(generator);
						}}

						template<typename T, typename std::enable_if<!std::is_floating_point<T>::value, int>::type = 0>
						__global__
						inline T random(T lower, T upper, uint64_t seed = -1) {{
						// Random integral value in range [lower, upper]
						return (T) random((double) (lower - (lower < 0 ? 1 : 0)), (double) upper + 1, seed);
						}}

						template<typename T>
						__device__
						inline auto {3}({4}) {{
							{6}
						}}

						template<typename T_DST, typename T_SRC>
						__global__
						void binaryFuncTrivial(T_DST *__restrict dstData,
										   const T_SRC **__restrict srcPointers,
										   int64_t numArrays,
										   int64_t size) {{
							const int64_t kernelIndex = blockDim.x * blockIdx.x
													   + threadIdx.x;

							if (kernelIndex < size) {{
								dstData[kernelIndex] = {3}({5});
							}}
						}})V0G0N",
							LIBRAPID_MAX_DIMS,
							CUDA_INCLUDE_DIRS,
							imp::complexHpp,
							kernel.name,
							args,
							indices,
							kernel.kernel);

						  static const std::vector<std::string> params = {
							"--disable-warnings",
							"-std=c++17",
							std::string("-I \"") + CUDA_INCLUDE_DIRS + "\""};

						  static jitify::JitCache kernelCache;
						  jitify::Program program = kernelCache.program(
							gpuKernel, imp::cudaHeaders, params);

						  int64_t threadsPerBlock, blocksPerGrid;

						  // Use 1 to 512 threads per block
						  if (end < 512) {
							  threadsPerBlock = end;
							  blocksPerGrid	  = 1;
						  } else {
							  threadsPerBlock = 512;
							  blocksPerGrid =
								ceil(double(end) / double(threadsPerBlock));
						  }

						  dim3 grid(blocksPerGrid);
						  dim3 block(threadsPerBlock);

#	ifdef LIBRAPID_CUDA_STREAM
						  jitifyCall(program.kernel("binaryFuncTrivial")
									   .instantiate(Type<TYPE>(), Type<TYPE>())
									   .configure(grid, block, 0, cudaStream)
									   .launch(dst.m_dataStart,
											   gpuPointers,
											   sizeof...(Pack),
											   end));
#	else  // LIBRAPID_CUDA_STREAM
						  jitifyCall(program.kernel("binaryFuncTrivial")
									   .instantiate(Type<TYPE>(), Type<TYPE>())
									   .configure(grid, block)
									   .launch(dst.m_dataStart,
											   gpuPointers,
											   sizeof...(Pack),
											   end));
#	endif // LIBRAPID_CUDA_STREAM
					  }
				  }
#else  // LIBRAPID_HAS_CUDA
				  } else {
					  throw std::runtime_error(
						"Cannot apply GPU kernel because CUDA was not enabled");
				  }
#endif // LIBRAPID_HAS_CUDA
			  },
			  dst.m_dataStart);

			return dst;
		}

		Array

		operator-() const;

		Array operator+(const Array &other) const;
		Array operator-(const Array &other) const;
		Array operator*(const Array &other) const;
		Array operator/(const Array &other) const;

		template<typename T>
		inline Array operator+(const T &other) const {
			return *this + Array(other);
		}

		template<typename T>
		inline Array operator-(const T &other) const {
			return *this - Array(other);
		}

		template<typename T>
		inline Array operator*(const T &other) const {
			return *this * Array(other);
		}

		template<typename T>
		inline Array operator/(const T &other) const {
			return *this / Array(other);
		}

		void transpose(const Extent &order = Extent());

		[[nodiscard]] inline Array transposed(const Extent &order = Extent()) {
			auto res = clone();
			res.transpose(order);
			return res;
		}

		[[nodiscard]] Array dot(const Array &other) const;

		[[nodiscard]] inline std::string str(int64_t indent	 = 0,
											 bool showCommas = false) const {
			static int64_t tmpRows, tmpCols;
			return str(indent, showCommas, tmpRows, tmpCols);
		}

		[[nodiscard]] std::string str(int64_t indent, bool showCommas,
									  int64_t &printedRows,
									  int64_t &printedCols) const;

		template<typename FUNC>
		static inline void
		applyUnaryOp(Array &dst, const Array &src, const FUNC &operation,
					 bool permitInvalid = false, int64_t dstOffset = 0) {
			// Operate on one array and store the result in another array

			if (!permitInvalid &&
				(dst.m_references == nullptr || dst.m_extent != src.m_extent)) {
				throw std::invalid_argument("Cannot operate on array with " +
											src.m_extent.

											str()

											+ " and store the result in " +
											dst.m_extent.

											str()

				);
			}

			auto dstPtr = dst.createRaw();
			auto srcPtr = src.createRaw();
			auto size	= src.m_extent.size();

			if (dstOffset) {
				dstPtr.data = std::visit(
				  [&](auto *data) -> RawArrayData { return data + dstOffset; },
				  dstPtr.data);
			}

			if (!permitInvalid && dst.m_stride.isTrivial() &&
				dst.m_stride.isContiguous() && src.m_stride.isTrivial() &&
				src.m_stride.isContiguous()) {
				// Trivial
				imp::multiarrayUnaryOpTrivial(dstPtr, srcPtr, size, operation);
			} else {
				// Not trivial, so use advanced method
				imp::multiarrayUnaryOpComplex(dstPtr,
											  srcPtr,
											  size,
											  src.m_extent,
											  dst.m_stride,
											  src.m_stride,
											  operation,
											  dst.m_stride.

												isTrivial() &&

												dst.m_stride.

												isContiguous()

				);
			}

			dst.m_isScalar = src.m_isScalar;
		}

		template<typename FUNC>
		static inline Array
		applyUnaryOp(const Array &src, const FUNC &operation,
					 bool permitInvalid = false, int64_t dstOffset = 0) {
			// Operate on a single array and return a new array

			Array dst(src.m_extent, src.m_dtype, src.m_location);

			auto dstPtr = dst.createRaw();
			auto srcPtr = src.createRaw();
			auto size	= src.m_extent.size();

			if (dstOffset) {
				dstPtr.data = std::visit(
				  [&](auto *data) -> RawArrayData { return data + dstOffset; },
				  dstPtr.data);
			}

			if (!permitInvalid && dst.m_stride.isTrivial() &&
				dst.m_stride.isContiguous() && src.m_stride.isTrivial() &&
				src.m_stride.isContiguous()) {
				// Trivial
				imp::multiarrayUnaryOpTrivial(dstPtr, srcPtr, size, operation);
			} else {
				// Not trivial, so use advanced method
				imp::multiarrayUnaryOpComplex(dstPtr,
											  srcPtr,
											  size,
											  src.m_extent,
											  dst.m_stride,
											  src.m_stride,
											  operation,
											  dst.m_stride.isTrivial() &&
												dst.m_stride.isContiguous());
			}

			dst.m_isScalar = src.m_isScalar;

			return dst;
		}

		template<typename FUNC>
		[[nodiscard]] static inline Array applyUnaryOp(Array &src,
													   const FUNC &operation) {
			// Operate on one array and store the result in another array

			if (src.m_references == nullptr) {
				throw std::invalid_argument(
				  "Cannot operate on an "
				  "uninitialized array");
			}

			Array dst(src.m_extent, src.m_dtype, src.m_location);
			auto srcPtr = src.createRaw();
			auto dstPtr = dst.createRaw();
			auto size	= src.m_extent.size();

			if (src.m_stride.isTrivial() && src.m_stride.isContiguous()) {
				// Trivial
				imp::multiarrayUnaryOpTrivial(dstPtr, srcPtr, size, operation);
			} else {
				// Not trivial, so use advanced method
				imp::multiarrayUnaryOpComplex(dstPtr,
											  srcPtr,
											  size,
											  dst.m_extent,
											  dst.m_stride,
											  src.m_stride,
											  operation,
											  true);
			}

			dst.m_isScalar = src.m_isScalar;

			return dst;
		}

		template<class FUNC>
		static inline void applyBinaryOp(Array &dst, const Array &srcA,
										 const Array &srcB,
										 const FUNC &operation,
										 bool permitInvalid = false,
										 bool permitVectorize = true) {
			// Operate on two arrays and store the result in another array

			if (!permitInvalid && (!srcA.m_isScalar && !srcB.m_isScalar &&
								   srcA.m_extent != srcB.m_extent)) {
				throw std::invalid_argument(
				  "Cannot operate on two arrays with " + srcA.m_extent.str() +
				  " and " + srcA.m_extent.str());
			}

			if (!permitInvalid && (dst.m_references == nullptr ||
								   dst.m_extent != srcA.m_extent)) {
				throw std::invalid_argument(
				  "Cannot operate on two arrays with " + srcA.m_extent.str() +
				  " and store the result in " + dst.m_extent.str());
			}

			auto ptrSrcA = srcA.createRaw();
			auto ptrSrcB = srcB.createRaw();
			auto ptrDst	 = dst.createRaw();
			auto size	 = dst.m_extent.size();

			if ((srcA.m_stride.isTrivial() && srcA.m_stride.isContiguous() &&
				 srcB.m_stride.isTrivial() && srcB.m_stride.isContiguous()) ||
				(srcA.m_stride == srcB.m_stride)) {
				// Trivial
				imp::multiarrayBinaryOpTrivial(ptrDst,
											   ptrSrcA,
											   ptrSrcB,
											   srcA.m_isScalar,
											   srcB.m_isScalar,
											   size,
											   operation,
											   permitVectorize);

				// Update the result stride too
				dst.m_stride = srcA.m_isScalar ? srcB.m_stride : srcA.m_stride;
			} else {
				// Not trivial, so use advanced method
				imp::multiarrayBinaryOpComplex(ptrDst,
											   ptrSrcA,
											   ptrSrcB,
											   srcA.m_isScalar,
											   srcB.m_isScalar,
											   size,
											   dst.m_extent,
											   dst.m_stride,
											   srcA.m_stride,
											   srcB.m_stride,
											   operation);
			}

			if (srcA.m_isScalar && srcB.m_isScalar) { dst.m_isScalar = true; }
		}

		template<class FUNC>
		[[nodiscard]] static inline Array
		applyBinaryOp(const Array &srcA, const Array &srcB,
					  const FUNC &operation, bool permitInvalid = false,
					  bool permitVectorize = true) {
			// Operate on two arrays and store the result in another array

			if (!permitInvalid && !(srcA.m_isScalar || srcB.m_isScalar) &&
				srcA.m_extent != srcB.m_extent) {
				throw std::invalid_argument(
				  "Cannot operate on two arrays with " + srcA.m_extent.str() +
				  " and " + srcB.m_extent.str());
			}

			Accelerator newLoc = max(srcA.m_location, srcB.m_location);
			Datatype newType   = max(srcA.m_dtype, srcB.m_dtype);

			Array dst(
			  srcA.m_isScalar ? srcB.m_extent : srcA.m_extent, newType, newLoc);

			auto ptrSrcA = srcA.createRaw();
			auto ptrSrcB = srcB.createRaw();
			auto ptrDst	 = dst.createRaw();
			auto size	 = dst.m_extent.size();

			if ((srcA.m_stride.isTrivial() && srcA.m_stride.isContiguous() &&
				 srcB.m_stride.isTrivial() && srcB.m_stride.isContiguous()) ||
				(srcA.m_stride == srcB.m_stride)) {
				// Trivial
				imp::multiarrayBinaryOpTrivial(ptrDst,
											   ptrSrcA,
											   ptrSrcB,
											   srcA.m_isScalar,
											   srcB.m_isScalar,
											   size,
											   operation,
											   permitVectorize);

				// Update the result stride too
				dst.m_stride = srcA.m_isScalar ? srcB.m_stride : srcA.m_stride;
			} else {
				// Not trivial, so use advanced method
				imp::multiarrayBinaryOpComplex(ptrDst,
											   ptrSrcA,
											   ptrSrcB,
											   srcA.m_isScalar,
											   srcB.m_isScalar,
											   size,
											   dst.m_extent,
											   dst.m_stride,
											   srcA.m_stride,
											   srcB.m_stride,
											   operation);
			}

			if (srcA.m_isScalar && srcB.m_isScalar) { dst.m_isScalar = true; }

			return dst;
		}

		/**
		 * \rst
		 *
		 * Internal function to offset the memory pointer within the array
		 *
		 * \endrst
		 */
		inline void _offsetData(int64_t elems) {
			m_dataStart = std::visit(
			  [&](auto *data) -> RawArrayData { return data + elems; },
			  m_dataStart);
			m_stride.setTrivial(false);
			m_stride.setContiguity(false);
		}

		inline void _resetOffset(int64_t elems) {
			m_dataStart = std::visit(
			  [&](auto *data) -> RawArrayData { return data - elems; },
			  m_dataStart);
			m_stride.setTrivial(true);
			m_stride.setContiguity(true);
		}

		inline void _setStart(const RawArrayData &data) { m_dataStart = data; }

		inline void _setScalar(bool val) { m_isScalar = val; }

		[[nodiscard]] inline RawArrayData _dataStart() const {
			return m_dataStart;
		}

		inline int64_t refCount() const { return *m_references; }
		inline void _increment() const { increment(); }
		inline void _decrement() { decrement(); }

		[[nodiscard]] AIterator begin() const;
		[[nodiscard]] AIterator end() const;

	private:
		inline void initializeCudaStream() const {
#ifdef LIBRAPID_HAS_CUDA
#	ifdef LIBRAPID_CUDA_STREAM
			if (!streamCreated) {
				checkCudaErrors(cudaStreamCreateWithFlags(
				  &cudaStream, cudaStreamNonBlocking));
				streamCreated = true;
			}
#	endif // LIBRAPID_CUDA_STREAM
#endif	   // LIBRAPID_HAS_CUDA
		}

		inline void increment() const {
			if (m_references == nullptr) { return; }
			++(*m_references);
		}

		inline void decrement() {
			if (m_references == nullptr) { return; }
			--(*m_references);

			if (*m_references == 0) {
				// Delete data
				rawArrayFree(createRaw());
				delete m_references;
			}
		}

		void constructNew(const Extent &e, const Stride &s,
						  const Datatype &dtype, const Accelerator &location);

		void constructHollow(const Extent &e, const Stride &s,
							 const Datatype &dtype,
							 const Accelerator &location);

		[[nodiscard]] std::pair<int64_t, int64_t>
		stringifyFormatPreprocess(bool stripMiddle, bool autoStrip) const;

		std::string stringify(int64_t indent, bool showCommas, bool stripMiddle,
							  bool autoStrip,
							  std::pair<int64_t, int64_t> &longest,
							  int64_t &printedRows, int64_t &printedCols) const;

	private:
		Accelerator m_location = Accelerator::CPU;
		Datatype m_dtype	   = Datatype::NONE;

		RawArrayData m_dataStart;
		RawArrayData m_dataOrigin;

		// std::atomic to allow for multithreading, because multiple threads may
		// increment/decrement at the same clock cycle, resulting in values
		// being incorrect and errors turning up all over the place
		std::atomic<int64_t> *m_references = nullptr;

		Extent m_extent;
		Stride m_stride;

		bool m_isScalar = false; // Array is a scalar value
		bool m_isChild =
		  false; // Array is a direct subscript of another (e.g. x[0])
	};

	Array zerosLike(const Array &other);

	Array onesLike(const Array &other);

	template<typename T = double>
	inline Array randomLike(const Array &other, const T &min = 0,
							const T &max = 0, int64_t seed = -1) {
		return other.filledRandom(min, max, seed);
	}

	Array linear(double start, double end, int64_t num,
				 const Datatype &dtype	 = Datatype::FLOAT64,
				 const Accelerator &locn = Accelerator::CPU);

	Array linear(double start, double end, int64_t num,
				 const std::string &dtype,
				 const Accelerator &locn = Accelerator::CPU);

	Array linear(double start, double end, int64_t num, const Datatype &dtype,
				 const std::string &locn = "CPU");

	Array linear(double start, double end, int64_t num,
				 const std::string &dtype, const std::string &locn);

	Array range(double start,
				double end = std::numeric_limits<double>::infinity(),
				double inc = 1, const Datatype &dtype = Datatype::FLOAT64,
				const Accelerator &locn = Accelerator::CPU);

	Array range(double start, double end, double inc, const std::string &dtype,
				const Accelerator &locn = Accelerator::CPU);

	Array range(double start, double end, double inc, const Datatype &dtype,
				const std::string &locn = "CPU");

	Array range(double start, double end, double inc, const std::string &dtype,
				const std::string &locn);

	void negate(const Array &a, Array &res);

	void add(const Array &a, const Array &b, Array &res);

	void sub(const Array &a, const Array &b, Array &res);

	void mul(const Array &a, const Array &b, Array &res);

	void div(const Array &a, const Array &b, Array &res);

	[[nodiscard]] Array negate(const Array &a);

	[[nodiscard]] Array add(const Array &a, const Array &b);

	[[nodiscard]] Array sub(const Array &a, const Array &b);

	[[nodiscard]] Array mul(const Array &a, const Array &b);

	[[nodiscard]] Array div(const Array &a, const Array &b);

	template<typename T,
			 typename std::enable_if<std::is_scalar<T>::value, int>::type = 0>
	[[nodiscard]] inline Array operator+(T lhs, const Array &rhs) {
		return Array::applyBinaryOp(lhs, rhs, ops::Add());
	}

	template<typename T,
			 typename std::enable_if<std::is_scalar<T>::value, int>::type = 0>
	[[nodiscard]] inline Array operator-(T lhs, const Array &rhs) {
		return Array::applyBinaryOp(lhs, rhs, ops::Sub());
	}

	template<typename T,
			 typename std::enable_if<std::is_scalar<T>::value, int>::type = 0>
	[[nodiscard]] inline Array operator*(T lhs, const Array &rhs) {
		return Array::applyBinaryOp(lhs, rhs, ops::Mul());
	}

	template<typename T,
			 typename std::enable_if<std::is_scalar<T>::value, int>::type = 0>
	[[nodiscard]] inline Array operator/(T lhs, const Array &rhs) {
		return Array::applyBinaryOp(lhs, rhs, ops::Div());
	}

	/**
	 * \rst
	 *
	 * Given a list of arrays, join them together along an existing axis, and
	 *return the result.
	 *
	 * Parameters
	 * ----------
	 *
	 * arrays: vector, list, tuple
	 *		The list of arrays to concatenate
	 * axis = 0: integer
	 *		The axis along which to concatenate the arrays
	 *
	 * Returns
	 * -------
	 *
	 * stacked: Array
	 *		The stacked array
	 *
	 * .. Attention::
	 *		The arrays must have exactly the same extent, other than the
	 *dimension for the concatenating axis, which can be different for each
	 *array.
	 *
	 * Examples
	 * --------
	 *
	 * .. code-block:: python
	 *
	 *		# Create the example arrays
	 *		first = librapid.fromData(
	 *			[[1, 2, 3],
	 *			 [4, 5, 6]]
	 *		)
	 *
	 *		second = librapid.fromData(
	 *			[[7, 8, 9]]
	 *		)
	 *
	 *		# Concatenate the arrays and store the result
	 *		# (default axis is 0 -- stack along rows)
	 *		concatenated = librapid.concatenate((first, second))
	 *
	 *		print(concatenated)
	 *
	 *		"""
	 *		Gives:
	 *
	 *		[[1. 2. 3.]
	 * 		 [4. 5. 6.]
	 * 		 [7. 8. 9.]]
	 *		"""
	 *
	 * .. code-block:: python
	 *
	 *		# Create the example arrays
	 *		first = librapid.from_data(
	 *			[[1, 2, 3],
	 *			 [4, 5, 6],
	 *			 [7, 8, 9]]
	 *		)
	 *
	 *		second = librapid.Array(librapid.Extent(3, 2)).filledRandom(-1, 1)
	 *
	 *		# Concatenate the arrays and store the result
	 *		# (here, we are stacking on axis=1 -- along the columns)
	 *		concatenated = librapid.concatenate((first, second), 1)
	 *
	 *		print(concatenated)
	 *
	 *		"""
	 *		Could give:
	 *
	 *		[[ 1.             2.             3.             0.06195223331
	 *-0.1879928112 ] [ 4.             5.             6. 0.3225619793
	 *-0.1187359691 ] [ 7.             8.             9. 0.2637588978
	 *-0.1868984699 ]]
	 *		"""
	 *
	 * .. Hint::
	 *		The resulting array is contiguous in memory, meaning the
	 *resulting array can be operated on very quickly and efficiently. (see
	 *``Array.clone`` for more information)
	 *
	 * \endrst
	 */
	[[nodiscard]] Array concatenate(const std::vector<Array> &arrays,
									int64_t axis = 0);

	/**
	 * \rst
	 *
	 * Given a list of arrays, join them together along a new axis.
	 *
	 * Parameters
	 * ----------
	 *
	 * arrays: vector, list, tuple
	 *		The list of arrays to stack
	 * axis = 0: integer
	 *		The axis along which to stack the arrays
	 *
	 * Returns
	 * -------
	 * stacked: ndarray
	 *		An array with one more dimension than the input arrays
	 *
	 * .. Attention::
	 *		The arrays passed must all have exactly the same extent
	 *
	 * Examples
	 * --------
	 *
	 * .. code-block:: python
	 *
	 *		# Create the example arrays
	 *		first = librapid.from_data(
	 *			[1, 2, 3]
	 *		)
	 *
	 *		second = librapid.from_data(
	 *			[4, 5, 6]
	 *		)
	 *
	 *		# Stack the arrays and store the result
	 *		# (default axis is 0)
	 *		stacked = librapid.pack((first, second))
	 *
	 *		print(stacked)
	 *
	 *		"""
	 *		Gives:
	 *
	 *		[[1. 2. 3.]
	 * 		 [4. 5. 6.]]
	 *		"""
	 *
	 * .. code-block:: python
	 *
	 *		# Create the example arrays
	 *		first = librapid.from_data(
	 *			[[1, 2, 3],
	 *			 [4, 5, 6],
	 *			 [7, 8, 9]]
	 *		)
	 *
	 *		second = librapid.from_data(
	 *			[[10, 11, 12],
	 *			 [13, 14, 15],
	 *			 [16, 17, 18]]
	 *		)
	 *
	 *		# Stack the arrays and store the result
	 *		# (here, we are stacking on axis=1 -- along the columns)
	 *		stacked = librapid.stack((first, second), 1)
	 *
	 *		print(stacked)
	 *
	 *		"""
	 *		Gives:
	 *
	 *		[[[ 1.  2.  3.]
	 *		  [10. 11. 12.]]
	 *
	 *		 [[ 4.  5.  6.]
	 *		  [13. 14. 15.]]
	 *
	 *		 [[ 7.  8.  9.]
	 *		  [16. 17. 18.]]]
	 *		"""
	 *
	 * .. Hint::
	 *		The resulting array is contiguous in memory, meaning the
	 *resulting array can be operated on very quickly and efficiently. (see
	 *``ndarray.clone`` for more information)
	 *
	 * \endrst
	 */
	[[nodiscard]] Array stack(const std::vector<Array>(&arrays),
							  int64_t axis = 0);

	void dot(const Array &lhs, const Array &rhs, Array &res);

	[[nodiscard]] Array dot(const Array &lhs, const Array &rhs);

	[[nodiscard]] inline std::ostream &operator<<(std::ostream &os,
												  const Array &arr) {
		return os << arr.str();
	}

	/**
	 * \rst
	 *
	 * Warmup the CPU and GPU (if applicable) by performing a variety of
	 * operations with different datatypes repeatedly. This will ensure that
	 * most kernels are compiled and that the hardware is running at a high
	 * clock-speed
	 *
	 * Parameters
	 * ----------
	 *
	 * iters: Integer
	 * 		Number of iterations to perform for each operation
	 *
	 * \endrst
	 */
	inline void warmup(int64_t itersCPU = 10, int64_t itersGPU = -1) {
		std::cout << "Librapid Hardware Warmup\n";

		const auto types = {Datatype::INT64,
							Datatype::FLOAT32,
							Datatype::FLOAT64,
							Datatype::CFLOAT64};
#ifdef LIBRAPID_HAS_CUDA
		const auto locations = {Accelerator::CPU, Accelerator::GPU};
#else
		const auto locations = {Accelerator::CPU};
#endif // LIBRAPID_HAS_CUDA

#ifdef LIBRAPID_DEBUG
		std::cout << "Running in DEBUG mode. Using 100x100 matrices\n";
#else
		std::cout << "Running in RELEASE mode. Using 1000x1000 matrices\n";
#endif

		for (const auto &type : types) {
			for (const auto &location : locations) {
#ifdef LIBRAPID_DEBUG
				auto tmp = Array(Extent({100, 100}), type, location);
#else
				auto tmp = Array(Extent({1000, 1000}), type, location);
#endif

				std::cout << "Warming up " << acceleratorToString(location)
						  << " with " << datatypeToString(type);

				double start = seconds();

				int64_t iters;
				if (location == Accelerator::CPU) {
					iters = itersCPU;
				} else if (location == Accelerator::GPU) {
					if (itersGPU == -1) {
						iters = itersCPU;
					} else {
						iters = itersGPU;
					}
				} else {
					throw std::invalid_argument("Unknown Accelerator");
				}

				for (int64_t i = 0; i < iters; ++i) {
					tmp.fill(1);
					auto res = tmp.clone();

					res = tmp + res;
					res = tmp - res;
					res = tmp * res;
					res = tmp / res;

					res.fillRandom();
					auto dotted = res.dot(res);

					// Confuse the compiler
					res = dotted * dotted / tmp - dotted + res * tmp;

					if (location == Accelerator::GPU && i == 0) {
						std::cout << " || Kernels compiled";
					}

#ifdef LIBRAPID_HAS_CUDA
#	ifdef LIBRAPID_CUDA_STREAM
					if (location == Accelerator::GPU)
						cudaSafeCall(cudaStreamSynchronize(cudaStream));
#	endif
#endif
				}
				double end = seconds();

				std::cout << " || Completed in " << (end - start) * 1000
						  << " ms\n";
			}
		}
	}

	void optimiseThreads(double timePerThread = 1, bool verbose = false);
} // namespace librapid

#ifdef FMT_API
template<>
struct fmt::formatter<librapid::Array> {
	template<typename ParseContext>
	constexpr auto parse(ParseContext &ctx) {
		return ctx.begin();
	}

	template<typename FormatContext>
	auto format(const librapid::Array &arr, FormatContext &ctx) {
		return fmt::format_to(ctx.out(), arr.str());
	}
};
#endif // FMT_API

#endif // LIBRAPID_ARRAY