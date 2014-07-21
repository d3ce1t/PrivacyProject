#ifndef CONFIG_H
#define CONFIG_H

#include <QMutex>

extern void CoreLib_InitResources();

namespace dai {

class Config
{
public:
    static Config* getInstance();  
    void enableFilters();
    void disableFilters();
    bool isFiltersEnabled() const;

private:
    static Config* _instance;
    static QMutex  _mutex;

    Config();
    bool m_filters_enabled;
};

} // End namespace

#endif // CONFIG_H
