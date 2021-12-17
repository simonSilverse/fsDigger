#include <argp.h>
#include "fsDigger.h"
/*
 * Seems like <argp.h> not exactly a dependable and stable library with doc part displaying issue
 *

struct arguments{
	fsDigger* fsDigger_;
} arguments_;
*/
const char* argp_program_version = "fsDigger-v1";
const char* argp_program_bug_address = "simon.omni.acc@gmail.com";
static char doc[] = "The documentation for the fsDigger program. " \
					"For starters, look at the --mode option first. " \
					"Not much information for now, to be updated. ";
static char args_doc[] = "Example# fsDigger image=/dev/sdb1 mode=1 skip=4 rootPlus=2 ";
static struct argp_option argp_option_[] = {
	{"image",'i',"/dev/sdb1",OPTION_ARG_OPTIONAL
		,"Specify path to the image (device/file). "
	}
	,{"mode",'m',"1",OPTION_ARG_OPTIONAL
		,"Specify what to do when reading image. " \
		"\n1 = Examine the FSI. Updates '--fsi1', '--fsi2'. " \
		"\n2 = Compare difference between 2 FSI. " \
		"\n3 = Compare difference between 2 FSI. " \
			"\n\tRuns w/o exam mode prior execution. " \
		"\n4 = Search for last not null sector. " \
		"\n5 = Create an image starting from '--fsi1' to last not null sector. " \
		"\n6 = Create an image starting from '--fsi1' to last not null sector. " \
			"\n\tRuns w/o exam mode prior execution. "
	}
	,{"offset",'o',"4",OPTION_ARG_OPTIONAL
		,"Specify sector number to start from. " \
		"\nExam mode applicable."
	}
	,{"skip",'s',"4",OPTION_ALIAS}
	,{"rootPlus",'r',"2",OPTION_ARG_OPTIONAL
		,"Specify amount of additional sector to treat as a root directory. " \
		"\nThe '--pattern' uses this as part of search. " \
		"\nCompare mode applicable. "
	}
	,{"pattern"
		,'p'
		,"\"android|auto|backup|boot|efi|ini|lost|readme|recycle|sync|system|trash|user\""
		,OPTION_ARG_OPTIONAL
		,"\nSpecify pattern use to format the output of root directory. " \
		"\nChar is not case sensitive and should have compatibility with ECMAScript. " \
		"\nCompare mode applicable."
	}
	,{"pa_nonMatching"
		,'n'
		,"0"
		,OPTION_ARG_OPTIONAL
		,"Specify to output non-matching pattern. "
	}
	,{"backup",'b',"0",OPTION_ARG_OPTIONAL
		,"Specify to backup. " \
		"\nCompare mode applicable. "
	}
	,{"backupLessNum",'c',"0",OPTION_ARG_OPTIONAL
		,"Specify to backup of sector only if number lesser than this value. " \
		"\nCompare mode applicable. "
	}
	,{"update",'u',"0",OPTION_ARG_OPTIONAL
		,"Specify to update by making changes to any difference between FSI. " \
		"\nRequires '--backup' to be enabled. " \
		"\nCompare mode applicable. "
	}
	,{"fsi1",'y',"0",OPTION_ARG_OPTIONAL
		,"Specify sector number of the 1st FSI. " \
		"\nExam mode not applicable. "
	}
	,{"fsi2",'z',"0",OPTION_ARG_OPTIONAL
		,"Specify sector number of the 2nd FSI. " \
		"\nExam mode not applicable. "
	}
	,{"log",'l',"1",OPTION_ARG_OPTIONAL
		,"Specify verbosity level."
	}
	,{"quiet",'q',0,0
		,"Specify verbosity level=2."
	}
	,{0}	// ***Required, do not remove...
};

static error_t option_parser(int key, char* argv, struct argp_state* argp_state_){
	//struct arguments* arguments_ = argp_state_->input;
	//fsDigger* fsDigger_ =  argp_state_->input;
	stringstream ss_;
	ss_ << argv;

	switch(key){
		case 'i' :
			ss_ >> fsDigger::p_self->partition_img;
			break;
		case 'm' :
			ss_ >> fsDigger::p_self->mode;
			break;
		case 'o' :
		case 's' :
			ss_ >> fsDigger::p_self->dd_skip_sector;
			break;
		case 'r' :
			ss_ >> fsDigger::p_self->rootLen_plus;
			break;
		case 'p' :
			ss_ >> fsDigger_specimen::regex_rootPattern;
			break;
		case 'n' :
			ss_ >> fsDigger_specimen::regex_outputNonMatching;
			break;
		case 'b' :
			ss_ >> fsDigger::p_self->toBackup;
			break;
		case 'c' :
			ss_ >> fsDigger::p_self->backup_lessNum;
			break;
		case 'u' :
			ss_ >> fsDigger::p_self->toReflect;
			break;
		case 'y' :
			ss_ >> fsDigger::p_self->index_sector_fsi1;
			break;
		case 'z' :
			ss_ >> fsDigger::p_self->index_sector_fsi2;
			break;
		case 'l' :
			ss_ >> fsDigger::p_self->logHelper_.minLvl;
			break;
		case 'q' :
			fsDigger::p_self->logHelper_.minLvl = 2;
			break;
		default:
			// return ARGP_ERR_UNKNOWN;
			break;
	}
	return 0;
}

static struct argp argp_{
	argp_option_
	,option_parser
	,doc
	,args_doc
};
