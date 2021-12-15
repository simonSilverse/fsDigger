#include <string.h>

class charConverter{
public:
	char* p_char = NULL;

	charConverter(){}

	charConverter(char char_array[],int char_arrayLen,int mode=1){
		toHex(char_array,char_arrayLen);
	}

	charConverter& operator=(const charConverter& rhsObj){
		return assign(&rhsObj);
	}

	charConverter& assign(const charConverter* p_rhsObj){
		delete p_char;
		p_char = p_rhsObj->p_char;

		return *this;
	}

	char* const& toHex(char char_array[],int char_arrayLen){
		delete p_char;
		char singleHex[9];
		int singleHex_len;
		int written_len = 0;
		p_char = new char[char_arrayLen * 2 + 1];
		// p_char = (char* ) alloca(char_arrayLen * 2 + 1);
		// char char2_array[char_arrayLen*2 + 1];

		for(int i=0; i<char_arrayLen; i++){
			singleHex_len = sprintf(
				singleHex
				,"%02X"
				,char_array[i]
			);

			// printf("%s\n",singleHex);
			if(singleHex_len==8){
				*(singleHex + 1) = singleHex[7];
				*(singleHex) = singleHex[6];
				singleHex_len = 2;
			}

			memcpy(
				p_char + written_len
				,singleHex
				,singleHex_len
			);
			written_len += singleHex_len;
		}

		*(p_char + written_len) = '\0';

		return p_char;
	}

	~charConverter(){
		delete p_char;
	}
};
