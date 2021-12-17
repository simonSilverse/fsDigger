#include <sstream>
#include "fsDigger_err.cpp"
#include "../includes/logHelper.cpp"
#include "../includes/regexBox.cpp"

class fsDigger_specimen{
	int index = 0;
	ulong size;

	ulong* fsi1_sector_array;
	ulong* fsi2_sector_array;
	int* fsi2_matched_array;

public:
	inline static int regex_outputNonMatching = 0;
	inline static string regex_rootPattern = "android|auto|backup|boot" \
											"|efi|ini|lost|readme|recycle" \
											"|sync|system|trash|user";

	fsDigger_specimen() = delete;

	fsDigger_specimen(ulong size){
		this->size = size;
		fsi1_sector_array = new ulong[size];
		fsi2_sector_array = new ulong[size];
	}

	~fsDigger_specimen(){
		delete fsi1_sector_array;
		delete fsi2_sector_array;
	}

	int get_len(){
		return index;
	}

	int append(int sectorNum1,int sectorNum2){
		fsi1_sector_array[index] = sectorNum1;
		fsi2_sector_array[index] = sectorNum2;
		index++;

		return index;
	}

	void regex_hd(const char cmp_char_array[], int spec2sectorNum, logHelper* const logHelper_){
		//static regexBox regexBox1("^[[:w:]]{3,}[[:w:]~_-]*");
		static regexBox regexBox1("(?:[^[:w:]]|^)[[:w:]][[:w:]~_-]{2,}");
		//static regexBox regexBox1("(?:[^[:w:]]|^)[[:w:]]{3,}[[:w:]~_-]*");
		static regexBox regexBox2("(?:" + regex_rootPattern + ").*");
		static string string1;

		int matchedCount1 = 0;
		int matchedCount2 = 0;

		matchedCount1 = regexBox1.regex_chunk_hd(cmp_char_array,512,512);
		//matchedCount1 = regexBox1.regex_hd(cmp_char_array,512);
		string1 = regexBox1.get_matched();
		if(matchedCount1>0){
			matchedCount2 = regexBox2.regex_hd(string1);
			*logHelper_	<< "Specimen sector = " << spec2sectorNum
						<< ", patternHidden matched number = " << matchedCount1
						<< endl;
			logHelper_->out_hd(0);

			*logHelper_	<< "Filtered data:"
						<< endl << string1
						<< endl;
			logHelper_->out_hd(0);
		}

		if(matchedCount2>0){
			*logHelper_	<< "Specimen sector = " << spec2sectorNum
						<< ", pattern matched number = " << matchedCount2 << "/" << matchedCount1
						<< endl << "Filtered data:"
						<< endl << (regex_outputNonMatching>0&&logHelper_->minLvl>0 ? string1 : regexBox2.get_matched())
						<< endl;
			logHelper_->out_hd(1);
		}
	}

	char chunk(logHelper* const logHelper_, int numB4Newline){
		int i = 0;

		if(fsi1_sector_array[i]!='\x0'){
			*logHelper_ << fsi1_sector_array[i] << "!=" << fsi2_sector_array[i] << ' ';
		}
		i++;

		while(i<index){
			if(i%numB4Newline==0){
				*logHelper_ << endl;
			}

			*logHelper_ << fsi1_sector_array[i] << "!=" << fsi2_sector_array[i] << ' ';
			i++;
		}

		return '\x0';
	}

	ulong get_size(){
		return size;
	}
};
