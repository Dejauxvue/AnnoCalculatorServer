#include <string>

namespace reader
{


class version
{
public:
	static const std::string VERSION_TAG;

	static void check_and_log();

};

}