#ifndef AVSALARMMANAGE_REQUEST_H
#define AVSALARMMANAGE_REQUEST_H
#include <string>
namespace AVSAlarmManageLib {
    class Request
    {
    public:
        Request();
        ~Request();

    public:
        bool get(const char* url, std::string& response);
        bool post(const char* url, const char* data, std::string& response);

    };
}
#endif //AVSALARMMANAGE_REQUEST_H