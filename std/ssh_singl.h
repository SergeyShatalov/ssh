
#pragma once

namespace ssh
{
	extern ssh_u SSH singltons[32];
	void SSH ssh_singlton(ssh_u ptr, ssh_u index);

	template <typename T> class Singlton
	{
		Singlton(const Singlton& cpy);
		Singlton& operator = (const Singlton& cpy);
	public:
		Singlton() { ssh_singlton((ssh_u)new T(), T::singl_idx); }
		~Singlton() { delete (T*)singltons[T::singl_idx]; }
		T* operator->() const { return (T*)singltons[T::singl_idx]; }
		static T* Instance() { return (T*)singltons[T::singl_idx]; }
	};
}