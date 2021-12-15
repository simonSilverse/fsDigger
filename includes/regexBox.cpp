#include <string>
#include <regex>
#include <sstream>

using namespace std;
class regexBox{
	enum {NONE = 0 ,CHUNKING = 1};
	int mode = NONE;
	ostringstream oss;
	regex regex_;
	bool toLoop = true;

public:
	const char* p_char_pattern;

	regexBox() = delete;

	regexBox(const char* p_char_pattern, regex::flag_type f = regex_constants::ECMAScript | regex_constants::icase){
		this->p_char_pattern = p_char_pattern;
		regex_ = regex(p_char_pattern, f);
	}

	regexBox(string str_pattern, regex::flag_type f = regex_constants::ECMAScript | regex_constants::icase)
	:	regexBox(str_pattern.c_str(),f)
	{}

	int regex_single_hd(const char cmp_char_array[], int size){
		static cmatch cmatch_;

		regex_search(
			cmp_char_array
			,cmp_char_array + size
			,cmatch_
			,regex_
		);

		/*
		 @TODO maybe...
		 csub_match = cmatch_.begin();
		 */
		if(cmatch_.size()<1){
			return 0;
		}
		else{
			matched_hd(cmatch_.str());

			return 1;
		}
	}

	int regex_multi_hd(const char cmp_char_array[], int size){
		static int matchedCount;
		static cmatch cmatch_;

		matchedCount = 0;

		cregex_iterator cregex_iterator_(
			cmp_char_array
			,cmp_char_array + size
			,regex_
		);

		while(true){
			cmatch_ = *cregex_iterator_;
			if(cmatch_.size()<1){
				break;
			}

			matched_hd(cmatch_.str());
			cregex_iterator_++;
			matchedCount++;
		}

		return matchedCount;
	}

	int regex_hd(string& cmp_str){
		return regex_hd(cmp_str.c_str(),cmp_str.length());
	}

	int regex_hd(const char* p_cmp_char){
		return regex_hd(p_cmp_char,strlen(p_cmp_char));
	}

	int regex_hd(string& cmp_str, int size){
		return regex_hd(cmp_str.c_str(),size);
	}

	int regex_hd(const char* p_cmp_char, int size){
		if(mode==NONE){
			oss = ostringstream();
		}
		if(toLoop){
			return regex_multi_hd(p_cmp_char, size);
		}
		else{
			return regex_single_hd(p_cmp_char, size);
		}
	}

	int regex_chunk_hd(string& cmp_str, int incre){
		return regex_chunk_hd(cmp_str.c_str(),incre,cmp_str.length());
	}
	
	int regex_chunk_hd(string& cmp_str, int incre, int lenMax){
		return regex_chunk_hd(cmp_str.c_str(),incre,lenMax);
	}

	int regex_chunk_hd(const char* p_cmp_char, int incre, int lenMax){
		string string_;
		int matchedCount;
		int i;

		oss = ostringstream();
		mode = CHUNKING;
		matchedCount = 0;
		for(i=0; i<lenMax; i+=incre){
			if(i+incre>lenMax){
				incre = lenMax - i;
			}

			matchedCount += regex_hd(
				p_cmp_char + i
				,incre
			);
		}
		mode = NONE;

		return matchedCount;
	}

	void matched_hd(string string_){
		oss	<< '\t' << string_
			<< endl;
	}

	string get_matched(){
		return oss.str();
	}
	
	void set_toLoop(bool b){
		toLoop = b;
	}
};
