#ifndef MEX_TYPE_TRAITS_HPP
#define MEX_TYPE_TRAITS_HPP

#include <matrix.h>
#include <stdint.h>
#include <type_traits>

#include "MexMem.hpp"

template <typename T>
struct GetMexType {
	static constexpr mxClassID typeVal = mxUNKNOWN_CLASS;
};

template <> struct GetMexType < char16_t > { static constexpr mxClassID typeVal = ::mxCHAR_CLASS   ; };
template <> struct GetMexType < int8_t   > { static constexpr mxClassID typeVal = ::mxINT8_CLASS   ; };
template <> struct GetMexType < uint8_t  > { static constexpr mxClassID typeVal = ::mxUINT8_CLASS  ; };
template <> struct GetMexType < int16_t  > { static constexpr mxClassID typeVal = ::mxINT16_CLASS  ; };
template <> struct GetMexType < uint16_t > { static constexpr mxClassID typeVal = ::mxUINT16_CLASS ; };
template <> struct GetMexType < int32_t  > { static constexpr mxClassID typeVal = ::mxINT32_CLASS  ; };
template <> struct GetMexType < uint32_t > { static constexpr mxClassID typeVal = ::mxUINT32_CLASS ; };
template <> struct GetMexType < int64_t  > { static constexpr mxClassID typeVal = ::mxINT64_CLASS  ; };
template <> struct GetMexType < uint64_t > { static constexpr mxClassID typeVal = ::mxUINT64_CLASS ; };
template <> struct GetMexType < float    > { static constexpr mxClassID typeVal = ::mxSINGLE_CLASS ; };
template <> struct GetMexType < double   > { static constexpr mxClassID typeVal = ::mxDOUBLE_CLASS ; };

template <typename T, class Al>              struct GetMexType<MexVector<T, Al> >                   { static constexpr uint32_t typeVal = GetMexType<T>::typeVal; };
template <typename T, class AlSub, class Al> struct GetMexType<MexVector<MexVector<T, AlSub>, Al> > { static constexpr uint32_t typeVal = mxCELL_CLASS; };

// Type Traits extraction for Vectors
template <typename T, typename B = void> 
	struct isMexVector 
		{ static constexpr bool value = false; };
template <typename T, class Al> 
	struct isMexVector<MexVector<T, Al>, typename std::enable_if<std::is_arithmetic<T>::value >::type > 
		{ static constexpr bool value = true; typedef T type; };

// Type Traits extraction for Vector of Vectors
template <typename T, class B = void>
	struct isMexVectVector 
		{ static constexpr bool value = false; };
template <typename T, class Al>
	struct isMexVectVector<MexVector<T, Al>, typename std::enable_if<isMexVector<T>::value>::type > 
	{
		static constexpr bool value = true;
		typedef typename isMexVector<T>::type type;
		typedef T elemType;
	};
template <typename T, class Al>
	struct isMexVectVector<MexVector<T, Al>, typename std::enable_if<isMexVectVector<T>::value>::type >
	{
		static constexpr bool value = true;
		typedef typename isMexVectVector<T>::type type;
		typedef T elemType;
	};

// Type Traits extraction for Matrix
template <typename T, typename B = void> 
	struct isMexMatrixBasic
		{ static constexpr bool value = false; };
template <typename T, class Al> 
	struct isMexMatrixBasic<MexMatrix<T, Al>, typename std::enable_if<std::is_arithmetic<T>::value>::type>
		{ static constexpr bool value = true; typedef T type; };
template <typename T> 
	struct isMexMatrix : public isMexMatrixBasic<typename std::decay<T>::type> {};

inline bool isMexVectorType(mxClassID ClassIDin) {
	switch (ClassIDin) {
		case mxINT8_CLASS   :
		case mxUINT8_CLASS  :
		case mxINT16_CLASS  :
		case mxUINT16_CLASS :
		case mxINT32_CLASS  :
		case mxUINT32_CLASS :
		case mxINT64_CLASS  :
		case mxUINT64_CLASS :
		case mxSINGLE_CLASS :
		case mxDOUBLE_CLASS :
			return true;
			break;
		default:
			return false;
	}
}

// Default Type Checking
template<typename T = void, class B = void>
struct FieldInfo {
	static inline bool CheckType(const mxArray* InputmxArray) {
		return false;
	}
	static inline uint32_t getSize(const mxArray* InputmxArray) {
		return 0;
	}
};

// Non Type Checking
template<>
struct FieldInfo<void> {
	static inline bool CheckType(const mxArray* InputmxArray) {
		return true;
	}
	static inline uint32_t getSize(const mxArray* InputmxArray) {
		size_t NumElems = 0;

		// If array is non-empty, calculate size
		if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray)) {
			NumElems = mxGetNumberOfElements(InputmxArray);
		}
		return NumElems;
	}
};

// Type Checking for scalar types
template<typename T>
struct FieldInfo<T, typename std::enable_if<std::is_arithmetic<T>::value >::type> {
	static inline bool CheckType(const mxArray* InputmxArray) {
		return (InputmxArray == nullptr || mxIsEmpty(InputmxArray) || mxGetClassID(InputmxArray) == GetMexType<T>::typeVal);
	}
	static inline uint32_t getSize(const mxArray* InputmxArray) {
		size_t NumElems = 0;

		// If array is non-empty, calculate size
		if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray)) {
			NumElems = mxGetNumberOfElements(InputmxArray);
		}
		return NumElems;
	}

};

// Type Checking for Vector of Scalars
template<typename T>
struct FieldInfo<T, typename std::enable_if<isMexVector<T>::value>::type> {
	static inline bool CheckType(const mxArray* InputmxArray) {
		return (InputmxArray == nullptr
		        || mxIsEmpty(InputmxArray)
		        || mxGetNumberOfDimensions(InputmxArray) == 2  // Check if 2-D Array
		           && (mxGetN(InputmxArray) == 1 || mxGetM(InputmxArray) == 1)  // Check if 1-D
		           && mxGetClassID(InputmxArray) == GetMexType<typename isMexVector<T>::type>::typeVal); // Check Type
	}
	static inline uint32_t getSize(const mxArray* InputmxArray) {
		size_t NumElems = 0;

		// If array is non-empty, calculate size
		if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray)) {
			NumElems = mxGetNumberOfElements(InputmxArray);
		}
		return NumElems;
	}
};

// Type Checking for Matrix of Scalars
template<typename T>
struct FieldInfo<T, typename std::enable_if<isMexMatrix<T>::value>::type> {
	static inline bool CheckType(const mxArray* InputmxArray) {
		return (InputmxArray == nullptr
		        || mxIsEmpty(InputmxArray)
		        || mxGetNumberOfDimensions(InputmxArray) == 2
		           && mxGetClassID(InputmxArray) == GetMexType<typename isMexMatrix<T>::type>::typeVal);
	}
	static inline uint32_t getSize(const mxArray* InputmxArray, uint32_t Dimension=0) {
		// Ths function assumes tat InputmxArray represents a valid Matrix. If not
		// then the result is undefined. Validate using CheckType prior to calling
		// this function.

		uint32_t NumElems = 0;

		// If array is non-empty, calculate size
		if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray)) {
			auto ArrayDims = mxGetDimensions(InputmxArray);
			if (Dimension < 2)
				NumElems = ArrayDims[Dimension];
		}
		return NumElems;
	}
};

// Type Checking for Cell Array (Vector Tree / Vector of Vectors)
template<typename T>
struct FieldInfo<T, typename std::enable_if<isMexVectVector<T>::value>::type> {
	static inline bool CheckType(const mxArray* InputmxArray) {
		bool isValid = true;
		if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray) && mxIsCell(InputmxArray)) {
			const mxArray* * SubVectorArray = reinterpret_cast<const mxArray* *>(mxGetData(InputmxArray));
			size_t NSubElems = mxGetNumberOfElements(InputmxArray);
			// Validate each subvector
			for (int i = 0; i < NSubElems; ++i) {
				if (!FieldInfo<typename isMexVectVector<T>::elemType>::CheckType(SubVectorArray[i])) {
					isValid = false;
					break;
				}
			}
		}
		else if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray)) {
			isValid = false;
		}
		else {
			isValid = true;
		}
		return isValid;
	}
	static inline uint32_t getSize(const mxArray* InputmxArray) {
		size_t NumElems = 0;

		// If array is non-empty, calculate size
		if (InputmxArray != nullptr && !mxIsEmpty(InputmxArray)) {
			NumElems = mxGetNumberOfElements(InputmxArray);
		}
		return NumElems;
	}
};

#endif