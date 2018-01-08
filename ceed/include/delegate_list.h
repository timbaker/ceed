#pragma once

#include "delegate.h"

#include <QList>

namespace Lumix
{
template <typename T> class DelegateList;

template <typename R, typename... Args> class DelegateList<R(Args...)>
{
public:
        DelegateList()
                : m_delegates()
	{
	}

	template <typename C, R (C::*Function)(Args...)> void bind(C* instance)
	{
		Delegate<R(Args...)> cb;
		cb.template bind<C, Function>(instance);
                m_delegates.append(cb);
	}

	template <R (*Function)(Args...)> void bind()
	{
		Delegate<R(Args...)> cb;
		cb.template bind<Function>();
                m_delegates.append(cb);
	}

	template <typename C, R (C::*Function)(Args...)> void unbind(C* instance)
	{
		Delegate<R(Args...)> cb;
		cb.template bind<C, Function>(instance);
		for (int i = 0; i < m_delegates.size(); ++i)
		{
			if (m_delegates[i] == cb)
			{
                                m_delegates.removeAt(i);
				break;
			}
		}
	}

	void invoke(Args... args)
	{
		for (auto& i : m_delegates) i.invoke(args...);
	}

private:
        QList<Delegate<R(Args...)>> m_delegates;
};

} // namespace Lumix
