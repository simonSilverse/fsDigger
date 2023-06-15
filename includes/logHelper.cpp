#ifndef _LOG_HELPER
#define _LOG_HELPER	1

#include <sstream>
#include <iostream>
#include <functional>

using namespace std;
class logHelper_blank{};

class logHelper : public stringstream{
	typedef logHelper_blank blank;

	static const int info_cArrayLen = 100;
	inline static locale locale_;
	inline static logHelper_blank blank_;
	char info_cArray[info_cArrayLen];

public:
	int readLen;
	int minLvl = 1;

	logHelper(int minLvl = 1)
	: stringstream(){
		locale locale_num;
		char* LC_NUMERIC_ = getenv("LC_NUMERIC");
		if(LC_NUMERIC_!=NULL){
			locale_num = locale(locale_,LC_NUMERIC_,locale::numeric);
			imbue(locale_num);
		}
		this->minLvl = minLvl;
	}

	void out_hd(){
		while(true){
			read(info_cArray,info_cArrayLen);
			readLen = gcount();
			if(readLen==0){
				break;
			}

			fwrite(
				info_cArray
				,readLen
				,1
				,stdout
			);
		}

		*this = logHelper(minLvl);
	}

	void out_hd(int lvl){
		if(lvl>=minLvl){
			out_hd();
		}
	}

	void out_hd(int lvl, function<void()> callback_hd){
		if(lvl>=minLvl){
			callback_hd();
			out_hd();
		}
	}

	void debug_out(){
		cout	<< endl << "char read = " << gcount()
				<< endl << "current pos_write = " << tellp()
				<< endl << "current pos_read = " << tellg()
				<< endl << "locale = " << getloc().name()
				<< endl;
	}

	blank chunk(char char_array[], int numB4Newline){
		int i = 0;

		if(char_array[i]!='\x0'){
			(*this) << char_array[i];
		}

		i++;
		while(true){
			if(char_array[i]=='\x0'){
				break;
			}

			if(i%numB4Newline==0){
				(*this) << endl;
			}

			(*this) << char_array[i];
			i++;
		}

		return blank_;
	}

	blank chunk(int int_array[], int numB4Newline){
		int i = 0;

		if(int_array[i]!='\x0'){
			(*this) << int_array[i] << ' ';
		}

		i++;
		while(true){
			if(int_array[i]=='\x0'){
				break;
			}

			if(i%numB4Newline==0){
				(*this) << endl;
			}

			(*this) << int_array[i] << ' ';
			i++;
		}

		return blank_;
	}

	blank left_jtf(string string_, int leftNum){
		static char tmp_char_array[25];
		string format = "%'-" + to_string(leftNum) + "s";

		sprintf(tmp_char_array,format.c_str(),string_.c_str());
		(*this) << tmp_char_array;

		return blank_;
	}

	blank left_jtf(int int_array, int leftNum){
		static char tmp_char_array[25];
		string format = "%'-" + to_string(leftNum) + "d";

		sprintf(tmp_char_array,format.c_str(),int_array);
		(*this) << tmp_char_array;

		return blank_;
	}

	/*
	__ostream_type& operator<<(blank blank_){
		return *(tie());
	}
	*/
};

template<class _Traits>
inline basic_ostream<char, _Traits>&
operator<<(basic_ostream<char, _Traits>& __out, logHelper_blank obj)
{ return __out; }

#endif
