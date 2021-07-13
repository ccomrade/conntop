/**
 * @file
 * @brief EventWrapper class.
 */

#pragma once

#include <type_traits>
#include <utility>

/**
 * @brief Container for storing single object of any type.
 * It is similar to std::any from C++17, but optimized specially for use in the EventSystem. Its code is based on the
 * idea used in implementation of std::any in GNU Standard C++ Library (libstdc++). Unlike std::any this container uses
 * directly global event IDs instead of RTTI for obtaining type of stored object.
 */
class EventWrapper
{
	union StorageUnion
	{
		void *heap;
		std::aligned_storage_t<8 * sizeof (void*), alignof (void*)> stack;
	};

	enum EImplOperation
	{
		IMPL_ID,      //!< arg is pointer to int
		IMPL_GET,     //!< arg is pointer to object pointer
		IMPL_MOVE,    //!< arg is pointer to StorageUnion in destination
		IMPL_DESTROY  //!< arg is nullptr
	};

	using ImplFunction = void (*)(EventWrapper *self, EImplOperation operation, void *arg);

	template<class T>
	struct HeapImpl
	{
		static void Function(EventWrapper *self, EImplOperation operation, void *arg) noexcept
		{
			switch (operation)
			{
				case IMPL_ID:
				{
					int *pID = static_cast<int*>(arg);
					(*pID) = T::ID;
					break;
				}
				case IMPL_GET:
				{
					T **pObjectPtr = static_cast<T**>(arg);
					(*pObjectPtr) = static_cast<T*>(self->m_storage.heap);
					break;
				}
				case IMPL_MOVE:
				{
					StorageUnion *pStorage = static_cast<StorageUnion*>(arg);
					pStorage->heap = self->m_storage.heap;
					self->m_storage.heap = nullptr;
					break;
				}
				case IMPL_DESTROY:
				{
					delete static_cast<T*>(self->m_storage.heap);
					self->m_storage.heap = nullptr;
					break;
				}
			}
		}

		template<class U>
		static void Create(StorageUnion *storage, U && object)
		{
			storage->heap = new T(std::forward<U>(object));
		}

		template<class... Args>
		static void Create(StorageUnion *storage, Args &&... args)
		{
			storage->heap = new T(std::forward<Args>(args)...);
		}
	};

	template<class T>
	struct StackImpl
	{
		static void Function(EventWrapper *self, EImplOperation operation, void *arg) noexcept
		{
			switch (operation)
			{
				case IMPL_ID:
				{
					int *pID = static_cast<int*>(arg);
					(*pID) = T::ID;
					break;
				}
				case IMPL_GET:
				{
					T **pObjectPtr = static_cast<T**>(arg);
					(*pObjectPtr) = reinterpret_cast<T*>(&self->m_storage.stack);
					break;
				}
				case IMPL_MOVE:
				{
					StorageUnion *pStorage = static_cast<StorageUnion*>(arg);
					T & object = reinterpret_cast<T&>(self->m_storage.stack);
					new (&pStorage->stack) T(std::move(object));
					reinterpret_cast<T*>(&self->m_storage.stack)->~T();
					break;
				}
				case IMPL_DESTROY:
				{
					reinterpret_cast<T*>(&self->m_storage.stack)->~T();
					break;
				}
			}
		}

		template<class U>
		static void Create(StorageUnion *pStorage, U && object)
		{
			new (&pStorage->stack) T(std::forward<U>(object));
		}

		template<class... Args>
		static void Create(StorageUnion *pStorage, Args &&... args)
		{
			new (&pStorage->stack) T(std::forward<Args>(args)...);
		}
	};

	template<class T, typename IsSafe = std::is_nothrow_move_constructible<T>,
	         bool Fits = (sizeof (T) <= sizeof (StorageUnion::stack)) && (alignof (T) <= alignof (void*))>
	using UseStackStorage = std::integral_constant<bool, IsSafe::value && Fits>;

	template<class T>
	using Impl = std::conditional_t<UseStackStorage<T>::value, StackImpl<T>, HeapImpl<T>>;

	template<class T>
	static ImplFunction CreateImpl(StorageUnion *pStorage, T && object)
	{
		using ObjType = std::decay_t<T>;

		static_assert(std::is_move_constructible<ObjType>::value, "The type must be MoveConstructible");

		Impl<ObjType>::Create(pStorage, std::forward<T>(object));
		return Impl<ObjType>::Function;
	}

	template<class T, class... Args>
	static ImplFunction CreateImpl(StorageUnion *pStorage, Args &&... args)
	{
		using ObjType = std::decay_t<T>;

		static_assert(std::is_move_constructible<ObjType>::value, "The type must be MoveConstructible");

		Impl<ObjType>::Create(pStorage, std::forward<Args>(args)...);
		return Impl<ObjType>::Function;
	}

	StorageUnion m_storage;
	ImplFunction m_implFunction;

public:
	EventWrapper() noexcept
	: m_implFunction(nullptr)
	{
	}

	template<class T>
	explicit EventWrapper(T && object)
	{
		m_implFunction = CreateImpl<T>(&m_storage, std::forward<T>(object));
	}

	// no copy
	EventWrapper(const EventWrapper&) = delete;
	EventWrapper & operator=(const EventWrapper&) = delete;

	EventWrapper(EventWrapper && other) noexcept
	{
		if (other.empty())
		{
			m_implFunction = nullptr;
		}
		else
		{
			other.m_implFunction(&other, IMPL_MOVE, &m_storage);
			m_implFunction = other.m_implFunction;
			other.m_implFunction = nullptr;
		}
	}

	EventWrapper & operator=(EventWrapper && other) noexcept
	{
		if (other.empty())
		{
			reset();
		}
		else if (this != &other)
		{
			reset();
			other.m_implFunction(&other, IMPL_MOVE, &m_storage);
			m_implFunction = other.m_implFunction;
			other.m_implFunction = nullptr;
		}
		return *this;
	}

	~EventWrapper()
	{
		reset();
	}

	bool empty() const noexcept
	{
		return m_implFunction == nullptr;
	}

	int getEventID() const noexcept
	{
		if (empty())
		{
			return -1;
		}
		else
		{
			int id;
			m_implFunction(const_cast<EventWrapper*>(this), IMPL_ID, &id);
			return id;
		}
	}

	template<class T>
	const T *get() const noexcept
	{
		if (empty() || T::ID != getEventID())
		{
			return nullptr;
		}
		else
		{
			T *pObject;
			m_implFunction(const_cast<EventWrapper*>(this), IMPL_GET, &pObject);
			return pObject;
		}
	}

	template<class T>
	T *get() noexcept
	{
		if (empty() || T::ID != getEventID())
		{
			return nullptr;
		}
		else
		{
			T *pObject;
			m_implFunction(this, IMPL_GET, &pObject);
			return pObject;
		}
	}

	template<class T, class... Args>
	T *emplace(Args &&... args)
	{
		reset();
		m_implFunction = CreateImpl<T>(&m_storage, std::forward<Args>(args)...);

		T *pObject;
		m_implFunction(this, IMPL_GET, &pObject);
		return pObject;
	}

	template<class T>
	T *store(T && object)
	{
		reset();
		m_implFunction = CreateImpl<T>(&m_storage, std::forward<T>(object));

		T *pObject;
		m_implFunction(this, IMPL_GET, &pObject);
		return pObject;
	}

	void reset() noexcept
	{
		if (!empty())
		{
			m_implFunction(this, IMPL_DESTROY, nullptr);
			m_implFunction = nullptr;
		}
	}
};
