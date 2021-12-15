#include "src/fsDigger.cpp"
#include "src/fsDigger_argp.cpp"

int main(int argc, char **argv) {
    fsDigger fsDigger_;
	fsDigger::p_self = &fsDigger_;
	// arguments_.fsDigger_ = &fsDigger_;

	argp_parse(&argp_,argc,argv,0,0,&fsDigger_);
	fsDigger_.ini();

	// ssize_t (fsDigger::* p_dd_write)(int fd, char const *buf, size_t size) = &fsDigger::dd_write;
    return 0;
}
