#pragma once

#include "nsContainer.h"



template<typename... TParams>
class nsTDelegateFunction
{
private:
	nsTDelegateFunction(const nsTDelegateFunction&) = delete;
	nsTDelegateFunction& operator=(const nsTDelegateFunction&) = delete;

public:
	nsTDelegateFunction() noexcept {}
	virtual void Execute(TParams... params) noexcept = 0;
	virtual bool IsMemberFunction() const noexcept { return false; }

};



template<typename... TParams>
class nsDelegateStaticFunction : public nsTDelegateFunction<TParams...>
{
public:
	typedef void(*Callback)(TParams...);

	Callback Call;


public:
	nsDelegateStaticFunction(Callback function) noexcept
		: Call(function)
	{
	}

	virtual void Execute(TParams... params) noexcept override
	{
		Call(std::forward<TParams>(params)...);
	}

};



template<typename T, typename... TParams>
class nsDelegateMemberFunction : public nsTDelegateFunction<TParams...>
{
public:
	typedef void(T::* Callback)(TParams...);

	T* Object;
	Callback Call;


public:
	nsDelegateMemberFunction(T* object, Callback function) noexcept
		: Object(object)
		, Call(function)
	{
	}

	virtual void Execute(TParams... params) noexcept override
	{
		(Object->*Call)(std::forward<TParams>(params)...);
	}

	virtual bool IsMemberFunction() const noexcept override
	{
		return true;
	}

};



template<typename... TParams>
class nsTDelegate
{
protected:
	nsTArray<nsTDelegateFunction<TParams...>*> InvokeList;


public:
	nsTDelegate() noexcept
	{
		InvokeList.Reserve(4);
	}


	virtual ~nsTDelegate() noexcept
	{
		UnbindAll();
	}


private:
	NS_NODISCARD int FindBoundStaticFunction(typename nsDelegateStaticFunction<TParams...>::Callback function) const noexcept
	{
		int index = NS_ARRAY_INDEX_INVALID;

		for (int i = 0; i < InvokeList.GetCount(); ++i)
		{
			if (InvokeList[i]->IsMemberFunction())
			{
				continue;
			}

			nsDelegateStaticFunction<TParams...>* check = dynamic_cast<nsDelegateStaticFunction<TParams...>*>(InvokeList[i]);

			if (check && check->Call == function)
			{
				index = i;
				break;
			}
		}

		return index;
	}

public:
	// Bind the static function if does not exist
	NS_INLINE void Bind(typename nsDelegateStaticFunction<TParams...>::Callback function) noexcept
	{
		const int index = FindBoundStaticFunction(function);

		if (index == NS_ARRAY_INDEX_INVALID)
		{
			InvokeList.Add(new nsDelegateStaticFunction<TParams...>(function));
		}
	}


	// Unbind the static function if exist
	NS_INLINE void Unbind(typename nsDelegateStaticFunction<TParams...>::Callback function) noexcept
	{
		const int index = FindBoundStaticFunction(function);

		if (index != NS_ARRAY_INDEX_INVALID)
		{
			if (InvokeList[index])
			{
				delete InvokeList[index];
				InvokeList[index] = nullptr;
			}

			InvokeList.RemoveAt(index, false);
		}
	}


private:
	template<typename T>
	NS_NODISCARD int FindBoundMemberFunction(T* object, typename nsDelegateMemberFunction<T, TParams...>::Callback function) const noexcept
	{
		int index = NS_ARRAY_INDEX_INVALID;

		for (int i = 0; i < InvokeList.GetCount(); ++i)
		{
			if (!InvokeList[i]->IsMemberFunction())
			{
				continue;
			}

			nsDelegateMemberFunction<T, TParams...>* check = dynamic_cast<nsDelegateMemberFunction<T, TParams...>*>(InvokeList[i]);

			if (check && check->Object == object && check->Call == function)
			{
				index = i;
				break;
			}
		}

		return index;
	}

public:
	// Bind member function if does not exists
	template<typename T>
	NS_INLINE void Bind(T* object, typename nsDelegateMemberFunction<T, TParams...>::Callback function) noexcept
	{
		const int index = FindBoundMemberFunction(object, function);

		if (index == NS_ARRAY_INDEX_INVALID)
		{
			InvokeList.Add(new nsDelegateMemberFunction<T, TParams...>(object, function));
		}
	}


	// Unbind member function if exists
	template<typename T>
	NS_INLINE void Unbind(T* object, typename nsDelegateMemberFunction<T, TParams...>::Callback function) noexcept
	{
		const int index = FindBoundMemberFunction(object, function);

		if (index != NS_ARRAY_INDEX_INVALID)
		{
			if (InvokeList[index])
			{
				delete InvokeList[index];
				InvokeList[index] = nullptr;
			}

			InvokeList.RemoveAt(index, false);
		}
	}

	// Remove all bound functions
	NS_INLINE void UnbindAll() noexcept
	{
		for (int i = 0; i < InvokeList.GetCount(); ++i)
		{
			if (InvokeList[i])
			{
				delete InvokeList[i];
				InvokeList[i] = nullptr;
			}
		}

		InvokeList.Clear(true);
	}

	// Check if any function bound
	NS_NODISCARD_INLINE bool IsBound() const noexcept
	{
		return !InvokeList.IsEmpty();
	}

};


#define NS_DELEGATE(delegateType) \
class delegateType : public nsTDelegate<> \
{ \
public: \
	inline void Broadcast() noexcept \
	{ \
		for (int i = 0; i < InvokeList.GetCount(); ++i) \
		{ \
			InvokeList[i]->Execute(); \
		} \
	} \
};


#define NS_DELEGATE_OneParam(delegateType, paramType, param) \
class delegateType : public nsTDelegate<paramType> \
{ \
public: \
	NS_INLINE void Broadcast(paramType param) noexcept \
	{ \
		for (int i = 0; i < InvokeList.GetCount(); ++i) \
		{ \
			InvokeList[i]->Execute(param); \
		} \
	} \
};


#define NS_DELEGATE_TwoParams(delegateType, paramType0, param0, paramType1, param1) \
class delegateType : public nsTDelegate<paramType0, paramType1> \
{ \
public: \
	NS_INLINE void Broadcast(paramType0 param0, paramType1 param1) noexcept \
	{ \
		for (int i = 0; i < InvokeList.GetCount(); ++i) \
		{ \
			InvokeList[i]->Execute(param0, param1); \
		} \
	} \
};


#define NS_DELEGATE_ThreeParams(delegateType, paramType0, param0, paramType1, param1, paramType2, param2) \
class delegateType : public nsTDelegate<paramType0, paramType1, paramType2> \
{ \
public: \
	NS_INLINE void Broadcast(paramType0 param0, paramType1 param1, paramType2 param2) noexcept \
	{ \
		for (int i = 0; i < InvokeList.GetCount(); ++i) \
		{ \
			InvokeList[i]->Execute(param0, param1, param2); \
		} \
	} \
};


#define NS_DELEGATE_FourParams(delegateType, paramType0, param0, paramType1, param1, paramType2, param2, paramType3, param3) \
class delegateType : public nsTDelegate<paramType0, paramType1, paramType2, paramType3> \
{ \
public: \
	NS_INLINE void Broadcast(paramType0 param0, paramType1 param1, paramType2 param2, paramType3 param3) noexcept \
	{ \
		for (int i = 0; i < InvokeList.GetCount(); ++i) \
		{ \
			InvokeList[i]->Execute(param0, param1, param2, param3); \
		} \
	} \
};


#define NS_DELEGATE_FiveParams(delegateType, paramType0, param0, paramType1, param1, paramType2, param2, paramType3, param3, paramType4, param4) \
class delegateType : public nsTDelegate<paramType0, paramType1, paramType2, paramType3, paramType4> \
{ \
public: \
	NS_INLINE void Broadcast(paramType0 param0, paramType1 param1, paramType2 param2, paramType3 param3, paramType4 param4) noexcept \
	{ \
		for (int i = 0; i < InvokeList.GetCount(); ++i) \
		{ \
			InvokeList[i]->Execute(param0, param1, param2, param3, param4); \
		} \
	} \
};
