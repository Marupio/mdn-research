#include <QLoggingCategory>
#include <QString>
#include <mdn/Logger.hpp>

namespace {

inline const char* msgTypeName(QtMsgType t)
{
    if (t == QtDebugMsg) {
        return "DEBUG";
    } else if (t == QtInfoMsg) {
        return "INFO";
    } else if (t == QtWarningMsg) {
        return "WARN";
    } else if (t == QtCriticalMsg) {
        return "CRIT";
    } else {
        return "FATAL";
    }
}


void qtMessageToMdnLogger(QtMsgType type, const QMessageLogContext& ctx, const QString& qmsg)
{
    mdn::Logger& log(mdn::Logger::instance());
    std::string msg(qmsg.toUtf8().constData());
    const char* file(ctx.file ? ctx.file : "");
    const char* func(ctx.function ? ctx.function : "");
    int line(ctx.line);

    if (type == QtDebugMsg) {
        log.debug(std::string("[QT][") + msgTypeName(type) + "] " + msg + " [" + file + ":" + std::to_string(line) + " " + func + "]");
    } else if (type == QtInfoMsg) {
        log.info(std::string("[QT][") + msgTypeName(type) + "] " + msg + " [" + file + ":" + std::to_string(line) + " " + func + "]");
    } else if (type == QtWarningMsg) {
        log.warn(std::string("[QT][") + msgTypeName(type) + "] " + msg + " [" + file + ":" + std::to_string(line) + " " + func + "]");
    } else if (type == QtCriticalMsg) {
        log.error(std::string("[QT][") + msgTypeName(type) + "] " + msg + " [" + file + ":" + std::to_string(line) + " " + func + "]");
    } else {
        log.error(std::string("[QT][") + msgTypeName(type) + "] " + msg + " [" + file + ":" + std::to_string(line) + " " + func + "]");
        std::abort();
    }
}


} // namespace


extern "C" void mdn_installQtMessageHandler()
{
    qInstallMessageHandler(&qtMessageToMdnLogger);
}
