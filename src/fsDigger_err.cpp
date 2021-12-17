#ifndef _FSI_PURIFIER_ERROR
#define _FSI_PURIFIER_ERROR	1

#include <string>
#include "../includes/logHelper.cpp"

class fsDigger_err{
	int num = 0;
	string info;

	logHelper logHelper_;

	fsDigger_err(){}

public:
	enum{
		INVALID_IMAGE = 1
		,INVALID_IMAGE_SIZE = 2
		,EXCEEDED_SECTOR_SLICE2 = 3
		,ERR_BACKUP_DIR = 4
		,ERR_DD_DIR = 5
		,ERR_FSI_LENGTH = 6
		,ERR_FROM_DD = 50
		,BAD_REG = 70
		,UNKNOWN = 99
	};

	fsDigger_err(int errnum, string info="")
	:num(errnum), info(info){
		get_error();
	}

	string get_error(int num = 0){
		if(num==0){
			num = this->num;
		}

		switch(num){
			case INVALID_IMAGE:
				logHelper_	<< "Unable to read image provided.";
				break;
			case INVALID_IMAGE_SIZE:
				logHelper_	<< "Invalid image provided."
							<< endl << "Max sector size for for image = " + info;
				break;	
			case EXCEEDED_SECTOR_SLICE2:
				logHelper_	<< "The filesystem cannot be purified."
							<< endl << "Possible reasones may be due to:"
							<< endl << "1) Size of partition may be too small."
							<< endl << "2) Offset provided too big.";
				break;
			case ERR_BACKUP_DIR:
				logHelper_	<< "Problem encountered while creating backup directory.";
				break;
			case ERR_DD_DIR:
				logHelper_	<< "Problem encountered while creating dd directory.";
				break;
			case ERR_FROM_DD:
				logHelper_	<< "An error has occured from fsDigger_static_compiled_dd.";
				break;
			case ERR_FSI_LENGTH:
				logHelper_	<< "Invalid length between fsi1 and fsi2";
				break;	
			case BAD_REG:
				logHelper_	<< "Regular expression badly formed."
							<< endl << info;
				break;
			
			case UNKNOWN:
				logHelper_	<< "Unknown error has occured.";
				break;
			default:
				logHelper_	<< "Error number not found";
		}
	
		logHelper_ << endl;
		return logHelper_.str();
	}

	int get_num(){
		return num;
	}

	void out_hd(){
		logHelper_.out_hd(logHelper_.minLvl);
	}
};

#endif
