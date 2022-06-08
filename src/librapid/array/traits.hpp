#pragma once

#include "../internal/config.hpp"
#include "../internal/forward.hpp"
#include "../internal/memUtils.hpp"

namespace librapid::internal {
	namespace flags {
		/**
		 * Flag Configuration:
		 *
		 * [0, 9]     -> Requirement flags
		 * [10, 31]   -> Operation type flags
		 * [32]       -> Unary operation
		 * [33]       -> Binary operation
		 * [34]       -> Matrix operation
		 */

		inline constexpr uint64_t Evaluated	   = 1ll << 0; // Result is already evaluated
		inline constexpr uint64_t RequireEval  = 1ll << 1; // Result must be evaluated
		inline constexpr uint64_t RequireInput = 1ll << 2; // Requires the entire array (not scalar)
		inline constexpr uint64_t HasCustomEval	 = 1ll << 3; // Has a custom eval function
		inline constexpr uint64_t SupportsScalar = 1ll << 4; // Has a custom eval function
		inline constexpr uint64_t SupportsPacket = 1ll << 5; // Has a custom eval function

		inline constexpr uint64_t Bitwise	 = 1ll << 10; // Bitwise functions
		inline constexpr uint64_t Arithmetic = 1ll << 11; // Arithmetic functions
		inline constexpr uint64_t Logical	 = 1ll << 12; // Logical functions
		inline constexpr uint64_t Matrix	 = 1ll << 13; // Matrix operation

		// Extract only operation information
		inline constexpr uint64_t OperationMask = 0b1111111111111111100000000000000;

		inline constexpr uint64_t PacketBitwise	   = 1ll << 14; // Packet needs bitwise
		inline constexpr uint64_t PacketArithmetic = 1ll << 15; // Packet needs arithmetic
		inline constexpr uint64_t PacketLogical	   = 1ll << 16; // Packet needs logical

		inline constexpr uint64_t ScalarBitwise	   = 1ll << 17; // Scalar needs bitwise
		inline constexpr uint64_t ScalarArithmetic = 1ll << 18; // Scalar needs arithmetic
		inline constexpr uint64_t ScalarLogical	   = 1ll << 19; // Scalar needs logical

		inline constexpr uint64_t Unary	 = 1ll << 32; // Operation takes one argument
		inline constexpr uint64_t Binary = 1ll << 33; // Operation takes two arguments

	} // namespace flags

	//------- Just a  Character -----------------------------------------------
	template<>
	struct traits<char> {
		static constexpr bool IsScalar		 = true;
		using Valid							 = std::true_type;
		using Scalar						 = char;
		using BaseScalar					 = char;
		using StorageType					 = memory::DenseStorage<char, device::CPU>;
		using Packet						 = std::false_type;
		using Device						 = device::CPU;
		static constexpr int64_t PacketWidth = 1;
		static constexpr char Name[]		 = "char";
		static constexpr uint64_t Flags =
		  flags::ScalarBitwise | flags::ScalarArithmetic | flags::ScalarLogical;
	};

	//------- Boolean ---------------------------------------------------------
	template<>
	struct traits<bool> {
		static constexpr bool IsScalar		 = true;
		using Valid							 = std::true_type;
		using Scalar						 = bool;
		using BaseScalar					 = uint64_t;
		using StorageType					 = memory::DenseStorage<bool, device::CPU>;
		using Packet						 = vcl::Vec512b;
		using Device						 = device::CPU;
		static constexpr int64_t PacketWidth = 512;
		static constexpr char Name[]		 = "bool";
		static constexpr uint64_t Flags		 = flags::PacketBitwise | flags::ScalarBitwise |
										  flags::ScalarArithmetic | flags::ScalarLogical;
	};

	//------- 8bit Signed Integer ---------------------------------------------
	template<>
	struct traits<int8_t> {
		static constexpr bool IsScalar		 = true;
		using Valid							 = std::true_type;
		using Scalar						 = int8_t;
		using BaseScalar					 = int8_t;
		using StorageType					 = memory::DenseStorage<int8_t, device::CPU>;
		using Packet						 = vcl::Vec64c;
		using Device						 = device::CPU;
		static constexpr int64_t PacketWidth = 64;
		static constexpr char Name[]		 = "int8_t";
		static constexpr uint64_t Flags		 = flags::PacketBitwise | flags::ScalarBitwise |
										  flags::PacketArithmetic | flags::ScalarArithmetic |
										  flags::PacketLogical | flags::ScalarLogical;
	};

	//------- 8bit Unsigned Integer -------------------------------------------
	template<>
	struct traits<uint8_t> {
		static constexpr bool IsScalar		 = true;
		using Valid							 = std::true_type;
		using Scalar						 = uint8_t;
		using BaseScalar					 = uint8_t;
		using StorageType					 = memory::DenseStorage<uint8_t>;
		using Packet						 = vcl::Vec64uc;
		using Device						 = device::CPU;
		static constexpr int64_t PacketWidth = 64;
		static constexpr char Name[]		 = "uint8_t";
		static constexpr uint64_t Flags		 = flags::PacketBitwise | flags::ScalarBitwise |
										  flags::PacketArithmetic | flags::ScalarArithmetic |
										  flags::PacketLogical | flags::ScalarLogical;
	};

	//------- 16bit Signed Integer --------------------------------------------
	template<>
	struct traits<int16_t> {
		static constexpr bool IsScalar		 = true;
		using Valid							 = std::true_type;
		using Scalar						 = int16_t;
		using BaseScalar					 = int16_t;
		using StorageType					 = memory::DenseStorage<int16_t>;
		using Packet						 = vcl::Vec32s;
		using Device						 = device::CPU;
		static constexpr int64_t PacketWidth = 32;
		static constexpr char Name[]		 = "int16_t";
		static constexpr uint64_t Flags		 = flags::PacketBitwise | flags::ScalarBitwise |
										  flags::PacketArithmetic | flags::ScalarArithmetic |
										  flags::PacketLogical | flags::ScalarLogical;
	};

	//------- 16bit Unsigned Integer ------------------------------------------
	template<>
	struct traits<uint16_t> {
		static constexpr bool IsScalar		 = true;
		using Valid							 = std::true_type;
		using Scalar						 = uint16_t;
		using BaseScalar					 = uint16_t;
		using StorageType					 = memory::DenseStorage<uint16_t>;
		using Packet						 = vcl::Vec32us;
		using Device						 = device::CPU;
		static constexpr int64_t PacketWidth = 32;
		static constexpr char Name[]		 = "uint16_t";
		static constexpr uint64_t Flags		 = flags::PacketBitwise | flags::ScalarBitwise |
										  flags::PacketArithmetic | flags::ScalarArithmetic |
										  flags::PacketLogical | flags::ScalarLogical;
	};

	//------- 32bit Signed Integer --------------------------------------------
	template<>
	struct traits<int32_t> {
		static constexpr bool IsScalar		 = true;
		using Valid							 = std::true_type;
		using Scalar						 = int32_t;
		using BaseScalar					 = int32_t;
		using StorageType					 = memory::DenseStorage<int32_t>;
		using Packet						 = vcl::Vec8i;
		using Device						 = device::CPU;
		static constexpr int64_t PacketWidth = 8;
		static constexpr char Name[]		 = "int32_t";
		static constexpr uint64_t Flags		 = flags::PacketBitwise | flags::ScalarBitwise |
										  flags::PacketArithmetic | flags::ScalarArithmetic |
										  flags::PacketLogical | flags::ScalarLogical;
	};

	//------- 32bit Unsigned Integer ------------------------------------------
	template<>
	struct traits<uint32_t> {
		static constexpr bool IsScalar		 = true;
		using Valid							 = std::true_type;
		using Scalar						 = uint32_t;
		using BaseScalar					 = uint32_t;
		using StorageType					 = memory::DenseStorage<uint32_t>;
		using Packet						 = vcl::Vec8ui;
		using Device						 = device::CPU;
		static constexpr int64_t PacketWidth = 4;
		static constexpr char Name[]		 = "uint32_t";
		static constexpr uint64_t Flags		 = flags::PacketBitwise | flags::ScalarBitwise |
										  flags::PacketArithmetic | flags::ScalarArithmetic |
										  flags::PacketLogical | flags::ScalarLogical;
	};

	//------- 64bit Signed Integer --------------------------------------------
	template<>
	struct traits<int64_t> {
		static constexpr bool IsScalar		 = true;
		using Valid							 = std::true_type;
		using Scalar						 = int64_t;
		using BaseScalar					 = int64_t;
		using StorageType					 = memory::DenseStorage<int64_t>;
		using Packet						 = vcl::Vec8q;
		using Device						 = device::CPU;
		static constexpr int64_t PacketWidth = 8;
		static constexpr char Name[]		 = "int64_t";
		static constexpr uint64_t Flags		 = flags::PacketBitwise | flags::ScalarBitwise |
										  flags::PacketArithmetic | flags::ScalarArithmetic |
										  flags::PacketLogical | flags::ScalarLogical;
	};

	//------- 64bit Unsigned Integer ------------------------------------------
	template<>
	struct traits<uint64_t> {
		static constexpr bool IsScalar		 = true;
		using Valid							 = std::true_type;
		using Scalar						 = uint64_t;
		using BaseScalar					 = uint64_t;
		using StorageType					 = memory::DenseStorage<uint64_t>;
		using Packet						 = vcl::Vec8uq;
		using Device						 = device::CPU;
		static constexpr int64_t PacketWidth = 8;
		static constexpr char Name[]		 = "uint64_t";
		static constexpr uint64_t Flags		 = flags::PacketBitwise | flags::ScalarBitwise |
										  flags::PacketArithmetic | flags::ScalarArithmetic |
										  flags::PacketLogical | flags::ScalarLogical;
	};

	//------- 32bit Floating Point --------------------------------------------
	template<>
	struct traits<float> {
		static constexpr bool IsScalar		 = true;
		using Valid							 = std::true_type;
		using Scalar						 = float;
		using BaseScalar					 = float;
		using StorageType					 = memory::DenseStorage<float>;
		using Packet						 = vcl::Vec16f;
		using Device						 = device::CPU;
		static constexpr int64_t PacketWidth = 16;
		static constexpr char Name[]		 = "float";
		static constexpr uint64_t Flags		 = flags::PacketArithmetic | flags::ScalarArithmetic |
										  flags::PacketLogical | flags::ScalarLogical;
	};

	//------- 64bit Floating Point --------------------------------------------
	template<>
	struct traits<double> {
		static constexpr bool IsScalar		 = true;
		using Valid							 = std::true_type;
		using Scalar						 = double;
		using BaseScalar					 = double;
		using StorageType					 = memory::DenseStorage<double>;
		using Packet						 = vcl::Vec8d;
		using Device						 = device::CPU;
		static constexpr int64_t PacketWidth = 8;
		static constexpr char Name[]		 = "double";
		static constexpr uint64_t Flags		 = flags::PacketArithmetic | flags::ScalarArithmetic |
										  flags::PacketLogical | flags::ScalarLogical;
	};

	template<typename LHS, typename RHS>
	struct PropagateDeviceType {
		using DeviceLHS = typename traits<LHS>::Device;
		using DeviceRHS = typename traits<RHS>::Device;
		using Device	= typename memory::PromoteDevice<DeviceLHS, DeviceRHS>::type;
	};

	template<typename LHS, typename RHS>
	struct ReturnType {
		using LhsType = LHS;
		using RhsType = RHS;
		using RetType = typename std::common_type<LhsType, RhsType>::type;
	};

	template<typename T>
	using StripQualifiers = typename std::remove_cv_t<typename std::remove_reference_t<T>>;
} // namespace librapid::internal