
#include <exception>
#include <string>

class ParseException : public std::exception
{
    public:

        ParseException(const std::string &reason) : m_reason(reason) { };
        ParseException(const char *reason) : m_reason(reason) { };

        virtual const char *what() const throw()
        {
            return m_reason.c_str();
        }

    
    private:

        std::string m_reason;

};