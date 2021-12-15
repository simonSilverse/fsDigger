#include <chrono>
#include "../includes/logHelper.cpp"

class fsDigger_progress{
protected:
	int milliseconds = 1000 * 60;
	bool toExit = false;
	ulong* p_n = NULL;
	ulong* p_n2 = NULL;

	ulong dd_bs;
	ulong* p_dd_skip_sector_fsi1 = NULL;
	ulong* p_dd_skip_sector_fsi2 = NULL;

public:
	logHelper logHelper_;

	virtual void ini(){
		int slice1_variance;
		int slice2_variance;
		chrono::milliseconds timespan(milliseconds);

		while(true){
			delay_hd(timespan);
			if(toExit){
				break;
			}
			else if(p_n2==NULL){
				continue;
			}

			slice1_variance = *p_n2 / dd_bs;
			slice2_variance = *p_n / dd_bs;

			logHelper_	<< "\tLast comparision:"
						<< endl << "\tfsi1 sector = " << *p_dd_skip_sector_fsi1 << " + " << slice1_variance
						<< endl << "\tfsi2 sector = " << *p_dd_skip_sector_fsi2 << " + " << slice2_variance
						<< endl;
			logHelper_.out_hd(1);
		}
	}

	void set_cmpVariables(ulong* p_n, ulong* p_n2, ulong* p_dd_skip_sector_fsi1, ulong* p_dd_skip_sector_fsi2){
		this->p_n = p_n;
		this->p_n2 = p_n2;
		this->p_dd_skip_sector_fsi1 = p_dd_skip_sector_fsi1;
		this->p_dd_skip_sector_fsi2 = p_dd_skip_sector_fsi2;
	}

	void set_dd_bs(ulong dd_bs){
		this->dd_bs = dd_bs;
	}
	
	void set_toExit(bool b){
		toExit = b;
	}
	
	virtual void delay_hd(chrono::milliseconds timespan){}
};
