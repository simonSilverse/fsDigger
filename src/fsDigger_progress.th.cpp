#include <chrono>
#include <thread>
#include "fsDigger_progress.cpp"
// #include <signal.h>

class th_fsDigger_progress : public fsDigger_progress{
	thread thread_;
	bool isAttached = false;

public:
	void ini()
	override{
		auto f_th_cxt = [&]()->void{
			// sched_setaffinity();
			// pthread_attr_getaffinity_np()
			
			fsDigger_progress::ini();
		};

		thread_ = thread(f_th_cxt);
		isAttached = true;
	}

	void delay_hd(chrono::milliseconds timespan)
	override{
		this_thread::sleep_for(timespan);
	}

	void detach(){
		isAttached = false;
		thread_.detach();
	}

	void kill(){
		//killpg(0,SIGKILL);

		set_toExit(true);
		if(isAttached) detach();
	}
};
