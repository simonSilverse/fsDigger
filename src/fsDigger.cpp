#include <math.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include "fsDigger.h"
#include "fsDigger_err.cpp"
#include "fsDigger_progress.th.cpp"
#include "fsDigger_specimen.cpp"
#include "../includes/logHelper.cpp"
#include "../includes/charConverter.cpp"

class fsDigger{
	int shared_multiplier = 1;
	static const ulong cmpSize = 512;
	char null_array[cmpSize];
	ulong fsi_sectorLenMax;
	ulong fsi_sectorLen = 0;

	inline static const char dd_stdout[] = "dd_stdout";
	inline static size_t dd_writtenSize = 0;
	static const ulong dd_perArgSize = 50;
	static const ulong dd_countMax = 4096;
	static const ulong dd_bs = 512;
	static const ulong dd_argc_exam = 7;
	static const ulong dd_argc_backup = 9;
	static const ulong dd_argc_reflect = 10;
	char * argv1[dd_argc_exam];
	char * argv2[dd_argc_exam];
	char * argv3[dd_argc_backup];
	char * argv4[dd_argc_reflect];
	int dd_argv_dyn_pos;
	ulong dd_count;
	ulong dd_countByte;
	ulong dd_if_sectorMax;

	char imageSlice1_char_array[dd_bs * dd_countMax];	// Up to 2,097,152
	char imageSlice2_char_array[dd_bs * dd_countMax];	// Up to 2,097,152
	inline static char* p_imageSlice_char = NULL;

	th_fsDigger_progress th_fsDigger_progress_;

public:
	string partition_img = "/dev/sdb1";
	int mode = 1;
	ulong dd_skip_sector = 4;
	ulong rootLen_plus = 2;

	bool toBackup = false;
	ulong backup_lessNum = 0;
	bool toReflect = false;

	ulong index_sector_fsi1 = 0;
	ulong index_sector_fsi2 = 0;
	inline static fsDigger* p_self = NULL;

	logHelper logHelper_;

	~fsDigger(){
		th_fsDigger_progress_.kill();
		close(DD_STDOUT_FILENO);
		unlink(dd_stdout);
	}

	void ini(){
		for(ulong i=0; i<cmpSize; i++){
			null_array[i] = '\x0';
		}

		try{
			dd_if_sectorMax_hd();
			dd_stdout_hd();
			make_dd_argv_fixed(argv1);
			make_dd_argv_fixed(argv2);

			switch(mode){
				case 1 :
					examFSI_hd();
					break;
				case 2 :
					if(examFSI_hd()!=0){
						break;
					}
					/*
					 * Fall through here after getting value from above:
					 *		-backup_lessNum
					 * 		-index_sector_fsi1
					 *		-index_sector_fsi2
					*/
				case 3 :
					findSpecimenDiff();
					break;
				case 4 :
					lastNotNull_sector_hd();
					break;
				case 5 :
					examFSI_hd();
					/*
					 * Fall through here after getting value from above:
					 * 		-index_sector_fsi1
					*/
				case 6 :
					createNotNullImage(lastNotNull_sector_hd());
					break;
				default :
					examFSI_hd();
			}
		}
		catch(fsDigger_err& e){
			e.out_hd();
			exit(e.get_num());
		}
	}

	void createNotNullImage(ulong sectorNum){
		ulong sectorSize = sectorNum - index_sector_fsi1;
		string backupDir = backupDir_hd();
		string backupFileName = backupDir + "/" +
								to_string(index_sector_fsi1) + "-" + to_string(sectorNum) + ".fsip.img";

		logHelper_	<< "Creating image of sector amount = " << sectorNum << " to " << backupFileName
					<< endl << "This may take quite an amount of time depending on the image size..."
					<< endl;
		logHelper_.out_hd(1);

		//dd_multiplier(128);
		dd_multiplier(dd_countMax);
		dd_arg_backup_hd(
			argv3
			,index_sector_fsi1
			,sectorSize
			,backupFileName
		);
		dd_hd(dd_argc_backup,argv3,NULL);

		logHelper_	<< "Creating image completed, check using:"
					<< endl << "dd if=" << backupFileName << " bs=512 count=2 \\"
					<< endl << "skip=$((" << to_string(sectorSize) << "*512)) \\"
					<< endl << "iflag=skip_bytes status=none | hexdump -C "
					<< endl;
		logHelper_.out_hd(1);
	}

	int examFSI_hd(){
		th_fsDigger_progress_.set_dd_bs(dd_bs);
		th_fsDigger_progress_.logHelper_.minLvl = logHelper_.minLvl;
		th_fsDigger_progress_.ini();
		if(pickSlice_hd()==0){
			return 0;
		}
		else{
			logHelper_	<< "FSI not found..."
						<< endl;
			logHelper_.out_hd(1);

			return 1;
		}
	}

	int pickSlice_hd(){
		for(int i=12; i<32; i++){
			fsi_sectorLenMax = pow(2,i) / 512;
			dd_count = dd_countMax;
			if(dd_count>fsi_sectorLenMax){
				dd_count = fsi_sectorLenMax;
			}
			dd_countByte = dd_count * dd_bs;
			logHelper_	<< i << ") "
						<< "Trying FSI len = " << fsi_sectorLenMax
						<< " (" << (fsi_sectorLenMax * dd_bs) << ") "
						<< "with count = " << dd_count
						<< endl;
			logHelper_.out_hd(1);

			if(pickSlice2Sectors()==0){
				return 0;
			}
		}

		return 1;
	}

	int pickSlice1Sectors(){
		static ulong i;

		for(i=0; i<fsi_sectorLenMax; i+=dd_count){
			index_sector_fsi1 = dd_skip_sector + i;
			dd_arg_dyn_hd(argv1,index_sector_fsi1,dd_count);
			dd_hd(dd_argc_exam,argv1,imageSlice1_char_array);
			if(cmpPickedSectors()==0){
				return 0;
			}
		}

		return 1;
	}

	int pickSlice2Sectors(){
		static ulong i;

		for(i=0; i<fsi_sectorLenMax; i+=dd_count){
			index_sector_fsi2 = dd_skip_sector + i + fsi_sectorLenMax;
			if(index_sector_fsi2>=dd_if_sectorMax){
				throw fsDigger_err(fsDigger_err::EXCEEDED_SECTOR_SLICE2);
			}

			dd_arg_dyn_hd(argv1,index_sector_fsi2,dd_count);
			dd_hd(dd_argc_exam,argv1,imageSlice2_char_array);
			logHelper_	<< "\tPicking image slice 2 sector = " << index_sector_fsi2
						<< endl;
			logHelper_.out_hd(1);
			if(pickSlice1Sectors()==0){
				return 0;
			}
		}

		return 1;
	}

	int cmpPickedSectors(){
		int nullCount = 0;
		static ulong n, n2;
		static ulong slice1_variance, slice2_variance;
		static charConverter charConverter_;
		static char cmp1_char_array[cmpSize];
		static char cmp2_char_array[cmpSize];
		th_fsDigger_progress_.set_cmpVariables(
			&n
			,&n2
			,&index_sector_fsi1
			,&index_sector_fsi2
		);

		for(n=0; n<dd_countByte; n+=cmpSize){
			memcpy(
				cmp2_char_array
				,(imageSlice2_char_array + n)
				,cmpSize
			);
			if(memcmp(cmp2_char_array,null_array,cmpSize)==0){
				nullCount++;
				continue;
			}

			for(n2=0; n2<dd_countByte; n2+=cmpSize){
				memcpy(
					cmp1_char_array
					,(imageSlice1_char_array + n2)
					,cmpSize
				);

				if(memcmp(cmp1_char_array,cmp2_char_array,cmpSize)==0){
					th_fsDigger_progress_.kill();
					slice1_variance = n2/dd_bs;
					slice2_variance = n/dd_bs;

					logHelper_	<< "Encountered number of null bytes group = " << nullCount
								<< endl << "First same " << cmpSize << " bytes:"
								<< endl << "\tfsi1 sector = " << (index_sector_fsi1 + slice1_variance)
								<< " (" << index_sector_fsi1 << " + " << slice1_variance << ")"
								<< endl << "\tfsi2 sector = " << (index_sector_fsi2 + slice2_variance)
								<< " (" << index_sector_fsi2 << " + " << slice2_variance << ")";

					index_sector_fsi1 += slice1_variance;
					index_sector_fsi2 += slice2_variance;
					backup_lessNum = index_sector_fsi2;

					logHelper_	<< endl << "\tSector difference = " << get_fsi_sectorLen()
								<< endl << "\tMatching:"
								<< endl << logHelper_.chunk(
												charConverter_.toHex(cmp2_char_array,cmpSize)
												,32 * 2
											)
								<< endl;
					logHelper_.out_hd(1);

					return 0;
				}
			}
		}

		return 1;
	}

	void findSpecimenDiff(){
		ulong i;
		ulong skip_sector_fsi1;
		ulong skip_sector_fsi2;
		bool hasExceededLen;

		fsi_sectorLen = get_fsi_sectorLen();
		if(fsi_sectorLen==0){
			throw fsDigger_err(fsDigger_err::ERR_FSI_LENGTH);
		}
		fsDigger_specimen fsDigger_specimen_(fsi_sectorLen + rootLen_plus);

		make_dd_argv_fixed(argv2);
		if(fsi_sectorLen>dd_countMax){
			// ***Loop will read excess sectors resulting in diff around the end
			dd_count = dd_countMax;
		}
		else{
			dd_count = fsi_sectorLen;
		}
		dd_countByte = dd_count * dd_bs;

		logHelper_	<< "Comparing sectors with count = " << dd_count
					<< endl;
		logHelper_.out_hd(1);

		// Starting from 2nd sector of specimen since first matching has already been found. 
		for(i=1; i<fsDigger_specimen_.get_size(); i+=dd_count){
			skip_sector_fsi1 = i + index_sector_fsi1;
			skip_sector_fsi2 = i + index_sector_fsi2;
			
			logHelper_	<< "Sectors begining = " << skip_sector_fsi2 << " (" << (skip_sector_fsi2 * dd_bs) << ")~"
						<< endl;
			logHelper_.out_hd(1);

			dd_arg_dyn_hd(argv1,skip_sector_fsi1,dd_count);
			dd_hd(dd_argc_exam,argv1,imageSlice1_char_array);
			dd_arg_dyn_hd(argv2,skip_sector_fsi2,dd_count);
			dd_hd(dd_argc_exam,argv2,imageSlice2_char_array);

			hasExceededLen = cmpSpecimenDiff(skip_sector_fsi1,fsDigger_specimen_);
			if(hasExceededLen){
				break;
			}
		}

		logHelper_	<< "Number of different sectors in " << cmpSize  << " bytes"
					<< " = " << fsDigger_specimen_.get_len()
					<< endl << "Sectors different:"
					<< endl << '\t' << fsDigger_specimen_.chunk(&logHelper_,10)
					<< endl;
		logHelper_.out_hd(1);
	}

	int cmpSpecimenDiff(ulong sectorNum, fsDigger_specimen& fsDigger_specimen_){
		static ulong n;
		static char cmp1_char_array[cmpSize];
		static char cmp2_char_array[cmpSize];
		static ulong sectorNum2;
		static string backupDir;
		static string backupFileName;

		for(n=0; n<dd_countByte; n+=cmpSize){
			memcpy(
				cmp1_char_array
				,(imageSlice1_char_array + n)
				,cmpSize
			);
			memcpy(
				cmp2_char_array
				,(imageSlice2_char_array + n)
				,cmpSize
			);

			if(memcmp(cmp1_char_array,cmp2_char_array,cmpSize)!=0){
				sectorNum2 = sectorNum + fsi_sectorLen;
				fsDigger_specimen_.append(sectorNum,sectorNum2);
				fsDigger_specimen_.regex_hd(cmp2_char_array,sectorNum2,&logHelper_);
				if(toBackup&&sectorNum<backup_lessNum){
					backupDir = backupDir_hd();
					backupFileName = backupDir + "/" + to_string(sectorNum) + ".fsip.img";
					dd_arg_backup_hd(
						argv3
						,sectorNum
						,1
						,backupFileName
					);
					dd_hd(dd_argc_backup,argv3,NULL);

					logHelper_	<< "Backup saved to " << backupFileName
								<< endl;
					logHelper_.out_hd(1);

					if(toReflect){
						dd_arg_reflect_hd(
							argv4
							,sectorNum2
							,1
							,partition_img
							,sectorNum
						);
						dd_hd(dd_argc_reflect,argv4,NULL);

						logHelper_	<< "Reflected sector " << sectorNum2 << " to " << sectorNum
									<< endl;
						logHelper_.out_hd(1);
					}
				}
			}

			sectorNum++;
			if(sectorNum>index_sector_fsi2+rootLen_plus){
				return 1;
			}
		}

		return 0;
	}

	string backupDir_hd(){
		static char tempDir[] = "fsDigger-XXXXXX";
		static bool isCreated = false;

		if(isCreated){
			return tempDir;
		}
		else if(mkdtemp(tempDir)==NULL){
			throw fsDigger_err(fsDigger_err::ERR_BACKUP_DIR);
		}
		else{
			make_dd_argv_fixed(argv3);
			make_dd_argv_fixed(argv4);
			isCreated = true;

			return tempDir;
		}
	}

	ulong lastNotNull_sector_hd(){
		static ulong i;
		static ulong lastNotNull_sector = 0;

		make_dd_argv_fixed(argv2);
		dd_multiplier(dd_countMax);
		logHelper_	<< "Searching for lastNotNull sector:"
					<< endl;
		logHelper_.out_hd(1);
		dd_count = dd_countMax;
		dd_countByte = dd_count * dd_bs;
		for(i=dd_if_sectorMax-dd_count; i>=0; i-=dd_count){
			index_sector_fsi2 = i;
			dd_arg_dyn_hd(argv2,index_sector_fsi2,dd_count);
			dd_hd(dd_argc_exam,argv2,imageSlice2_char_array);
			logHelper_	<< "Picking image sector = " << index_sector_fsi2 << "~"
						<< endl;
			logHelper_.out_hd(1);
			lastNotNull_sector = cmpNotNull();
			if(lastNotNull_sector>0){
				break;
			}
		}

		if(lastNotNull_sector==0){
			logHelper_	<< "Last not null " << cmpSize << " bytes not found."
						<< endl;
			logHelper_.out_hd(1);
		}

		return lastNotNull_sector;
	}

	ulong cmpNotNull(){
		static long n;
		static ulong lastNotNull;
		static charConverter charConverter_;
		static char cmp_char_array[cmpSize];

		for(n=dd_countByte-cmpSize; n>=0; n-=cmpSize){
			memcpy(
				cmp_char_array
				,(imageSlice2_char_array + n)
				,cmpSize
			);

			if(memcmp(cmp_char_array,null_array,cmpSize)==0){
				continue;
			}

			lastNotNull = index_sector_fsi2 + floor(n/dd_bs);
			logHelper_	<< "Last not null " << cmpSize << " bytes at sector = " << lastNotNull << " (" << (lastNotNull * dd_bs) << ")"
						<< endl << "\tMatching:"
						<< endl << logHelper_.chunk(
										charConverter_.toHex(cmp_char_array,cmpSize)
										,32 * 2
									)
						<< endl;
			logHelper_.out_hd(1);

			return lastNotNull;
		}

		return 0;
	}

	void restore_hd(){
		/*
		 * TODO
		 * This is just a tentative decision.
		 * Determination factors include:
		 * 		-Time to read through directory class usage.
		 * 		-Scripting out with Bash might be an alternative option.
		 */
	}

	void reformat_n_remap_hd(){
		/*
		 * TODO
		 * This is just a tentative decision.
		 * Determination factors include:
		 * 		-Time to read through directory class usage.
		 * 		-Having a more concrete knowledge of boot sector format pointing the location of FSI.
		 */
	}

	void make_dd_argv_fixed(char* argv[]){
		static string arg_if = "if=" + partition_img;
		static char arg1[dd_perArgSize] = "fsDigger_static_compiled_dd";
		static char arg2[dd_perArgSize] = "iflag=skip_bytes";
		static char arg3[dd_perArgSize] = "status=none";
		static char arg4[dd_perArgSize];
		static char arg5[dd_perArgSize];
		static char arg6[dd_perArgSize];
		static char arg7[dd_perArgSize];

		int i = 0;
		argv[i++] = arg1;
		argv[i++] = arg2;
		argv[i++] = arg3;
		argv[i++] = arg4;
		argv[i++] = arg5;
		argv[i++] = arg6;
		argv[i++] = arg7;
		dd_argv_dyn_pos = 4;

		strncpy(
			arg4
			,arg_if.c_str()
			,dd_perArgSize
		);
	}

	int dd_arg_dyn_hd(char* argv[], ulong& skip_sector, ulong count){
		static string arg_bs;
		static string arg_count;
		static string arg_skip;

		int i = dd_argv_dyn_pos;

		arg_bs = "bs=" + to_string(dd_bs * shared_multiplier);
		arg_count = "count=" + to_string((long) ceil((long double) count / shared_multiplier));
		arg_skip = "skip=" + to_string(dd_bs * skip_sector);

		strncpy(
			argv[i++]
			,arg_bs.c_str()
			,dd_perArgSize
		);

		strncpy(
			argv[i++]
			,arg_count.c_str()
			,dd_perArgSize
		);

		strncpy(
			argv[i++]
			,arg_skip.c_str()
			,dd_perArgSize
		);

		return i;
	}

	int dd_arg_backup_hd(char* argv[], ulong& skip_sector, ulong count, string& arg_of){
		static char arg9[dd_perArgSize] = "oflag=dsync";	// seek_bytes

		int i = dd_arg_extract_hd(argv,skip_sector,count,arg_of);

		argv[i++] = arg9;

		return i;
	}

	int dd_arg_reflect_hd(char* argv[], ulong& skip_sector, ulong count, string& arg_of, ulong& seek_sector){
		static string arg_seek;
		static char arg9[dd_perArgSize] = "oflag=dsync,seek_bytes";
		static char arg10[dd_perArgSize];
		
		int i = dd_arg_extract_hd(argv,skip_sector,count,arg_of);

		arg_seek = "seek=" + to_string(dd_bs * seek_sector);

		argv[i++] = arg9;
		argv[i++] = arg10;
		
		strncpy(
			arg10
			,arg_seek.c_str()
			,dd_perArgSize
		);

		return i;
	}

	int dd_arg_extract_hd(char* argv[], ulong& skip_sector, ulong count, string& arg_of){
		static char arg8[dd_perArgSize];

		int i = dd_arg_dyn_hd(argv,skip_sector,count);

		argv[i++] = arg8;

		strncpy(
			arg8
			,("of=" + arg_of).c_str()
			,dd_perArgSize
		);

		return i;
	}

	void dd_multiplier(int shared_multiplier){
		if(this->shared_multiplier<shared_multiplier){
			/*
			* Appearantly cannot free existing dd pointer.
			* Likely due to pointer not a reference to the original pointer at function local context.
			* Not modifying existing implementation.
			* Should not be a big problem since usage of this program rarely occurs.
			* 
			if(ibuf){
				free(ibuf);
			}
			if(obuf){
				free(obuf);
			}
			*/
			ibuf = NULL;
			obuf = NULL;
			this->shared_multiplier = shared_multiplier;
			logHelper_	<< "More resources to be allocated by dd, setting shared_multiplier = " << shared_multiplier
						<< endl;
			logHelper_.out_hd(0);
		}
	}

	void dd_hd(ulong dd_argc, char* argv[], char imageSlice_char_array[]){
		static int dd_result;

		r_full = 0;
		w_full = 0;
		w_bytes = 0;
		w_partial = 0;
		seek_bytes = 0;
		seek_records = 0;
		output_file = NULL;
		input_file = NULL;
		dd_writtenSize = 0;
		p_imageSlice_char = imageSlice_char_array;
		
		// debug_argv(dd_argc,argv);
		// exit(EXIT_SUCCESS);
		dd_result = dd_main(dd_argc,argv);
		if(dd_result!=0){
			throw fsDigger_err(fsDigger_err::ERR_FROM_DD);
		}
	}

	void dd_if_sectorMax_hd(){
		FILE * procFile = popen(
			("blockdev --getsz " + partition_img + " 2>&1").c_str()
			,"r"
		);
	
		if(procFile==NULL){
			throw fsDigger_err(fsDigger_err::INVALID_IMAGE);
		}
		else{
			// fread(procFile_in,25,1,procFile);
			fscanf(procFile,"%ld",&dd_if_sectorMax);
			pclose(procFile);
			if(dd_if_sectorMax==0){
				throw fsDigger_err(
					fsDigger_err::INVALID_IMAGE_SIZE
					,to_string(dd_if_sectorMax)
				);
			}
		}
	}
	
	void dd_stdout_hd(){
		DD_STDOUT_FILENO = open(
			dd_stdout
			, O_CREAT | O_APPEND | O_RDWR, S_IRWXU | S_IRWXG // O_SYNC
		);

		if(DD_STDOUT_FILENO==-1){
			throw fsDigger_err(fsDigger_err::ERR_DD_FILE);
		}
	}

	ulong get_fsi_sectorLen(){
		return index_sector_fsi2 - index_sector_fsi1;
	}

	static char* const& getRef_p_imageSlice_char(){
		return p_imageSlice_char;
	}

	static size_t& getRef_dd_writtenSize(){
		return dd_writtenSize;
	}

	void debug_argv(int argvc, char* argv[], bool toCombine = false){
		string string_;

		for(int i=0; i<argvc; i++){
			logHelper_	<< endl << "argv[" << i << "] addr = " << (argv + i)
						<< endl << "argv[" << i << "] val = " << *(argv + i)
						<< endl;

			if(toCombine){
				string_ += " " ;
				string_ += *(argv + i);
			}
		}

		if(string_.length()>0){
			logHelper_	<< string_
						<< endl;
		}
		logHelper_.out_hd(1);
		
	}
};

ssize_t fsDigger_write(int fd, char const* buf, size_t size){
	static size_t& dd_writtenSize = fsDigger::getRef_dd_writtenSize();
	static char* const& p_imageSlice_char = fsDigger::getRef_p_imageSlice_char();

	if(p_imageSlice_char==NULL){
		return write(fd,buf,size);
	}
	else{
		memcpy(
			p_imageSlice_char + dd_writtenSize
			,buf
			,size
		);
		dd_writtenSize += size;

		return size;
	}
}

void fsDigger_finish_up(void){
	/*
	 * 
	 * Appearantly not necessary most likely due to workings of C vs CPP compilers.
	 * Enabling it will cause error at part of execution.
	 * Anyways, resources will be automatically freed at end of program.
	 * 
	 * static char* const& p_imageSlice_char = fsDigger::getRef_p_imageSlice_char();
	 * cleanup_out();
	*/

	cleanup_in ();
	print_stats ();
	process_signals ();
}
