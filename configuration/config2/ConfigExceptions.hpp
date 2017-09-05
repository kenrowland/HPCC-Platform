
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


class ValueException : public std::exception
{
    public:

        ValueException(const std::string &reason) : m_reason(reason) { };
        ValueException(const char *reason) : m_reason(reason) { };

        virtual const char *what() const throw()
        {
            return m_reason.c_str();
        }

    
    private:

        std::string m_reason;

};


class ConfigException : public std::exception
{
public:

	ConfigException(const std::string &reason) : m_reason(reason) { };
	ConfigException(const char *reason) : m_reason(reason) { };

	virtual const char *what() const throw()
	{
		return m_reason.c_str();
	}


private:

	std::string m_reason;

};