#pragma once

#include <unordered_map>

template<typename T>
class A_Factory
{
public:

	virtual T* Create() = 0;
};

template<typename Product, typename Key>
class FactoryDictionary
{
public:

	typedef A_Factory<Product> FactoryInterface;

	static inline FactoryDictionary Instance;

	template<typename ConcreteProduct>
	class Factory : public FactoryInterface
	{
	public:

		Factory(Key key)
		{
			FactoryDictionary::Instance.Install(key, this);
		}
		
		Product* Create() override
		{
			return new ConcreteProduct();
		}	
	};

	FactoryInterface* FindFactory(Key key)
	{
		return _factories[key];
	}

private:

	void Install(Key key, FactoryInterface* factory)
	{
		_factories[key] = factory;
	}
	
private:

	std::unordered_map<Key, FactoryInterface*> _factories;
};