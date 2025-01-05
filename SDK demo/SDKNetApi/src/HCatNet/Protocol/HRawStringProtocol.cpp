

#include <Protocol/HRawStringProtocol.h>
#include <Tool/HTool.h>
#include <HCatTool.h>
#include <CatString.h>

#include <sstream>
#include <vector>
#include <fstream>


#define READ_FILE_MAX               (1024 * 10)


using namespace cat;


HRawStringProtocol::HRawStringProtocol(const EventCoreWeakPtr &core)
    : IProtocolBase(core)
    , sendfaild_(false)
    , negotiate_(false)
    , processing_(false)
{

}

HRawStringProtocol::~HRawStringProtocol()
{
    // 线程正在发送文件, 使其失败
    sendfaild_ = true;
    while (!threadRun_.expired() && !core_.expired()) {
        sendfaild_ = true;
        std::this_thread::yield();
    }
}

bool HRawStringProtocol::Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData)
{
    switch (cmd) {
    case kInit: {
        sendfaild_ = false;
        negotiate_ = false;
        delaySend_.clear();
        return true;
    } break;
    case kSendFaildAsk: {
        sendfaild_ = true;
        return true;
    } break;
    case kReset:
    case kHeartBeat: {
        return true;
    } break;
    case kConnect: {
        this->sendfaild_ = false;
        NotifyCmd(kNotofyConnect, data, userData);
        this->negotiate_ = true;
        return this->DelaySend();
    } break;
    case kReadData: {
        NotifyCmd(kReadyRead, data, userData);
        return true;
    } break;
    case kSendFile: {
        processing_ = userData.IsType<HSendProtocolFileInfo>();
        if (processing_ == false) {
            ParseLog(fmt::format("Send file type error[{}]", userData.Type().name()));
            return false;
        }

        if (userData.Cast<HSendProtocolFileInfo>().fileList.empty()) {
            processing_ = false;
            return true;
        }

        survivalTest_typePtr run(new survivalTest_type());
        threadRun_ = run;

        ICatEventApi::AddThread(core_, run, this, &HRawStringProtocol::ThreadSendFile, userData);
        return true;
    } break;
    case kSendSDK: {
        if (negotiate_ == false) {
            delaySend_.emplace_back(delaySend(cmd, data, userData));
            return true;
        }
        return this->SendData(data, userData);
    } break;
    default:
        break;
    }

    return false;
}

bool HRawStringProtocol::SendData(const HCatBuffer &data, const HCatAny &userData)
{
    NotifyCmd(kSendData, data, userData);
    return !sendfaild_;
}

bool HRawStringProtocol::ThreadSendFile(const HCatAny &userData)
{
    RallPoint<bool> rall(&processing_, true, false);
    if (sendfaild_) {
        return false;
    }

    HSendProtocolFileInfo fileList = userData.Cast<HSendProtocolFileInfo>();
    for (const auto &i : fileList.fileList) {
        if (sendfaild_) {
            return false;
        }

        std::fstream sendFile;
        sendFile.open(HTool::ToLocalString(i.filePath), std::ios::in | std::ios::binary);
        if (sendFile.is_open() == false) {
            ParseLog(fmt::format("open file[{}] faild", i.filePath));
            continue;
        }

        std::vector<char> data(READ_FILE_MAX);
        for (;;) {
            if (sendfaild_) {
                return false;
            }

            sendFile.read(data.data(), data.size());
            hint64 readSize = sendFile.gcount();
            if (readSize <= 0) {
                sendFile.close();
                if (readSize == 0) {
                    break;
                }
                ParseLog("Read file faild");
                return false;
            }

            if (SendData(HCatBuffer(data.data(), static_cast<std::size_t>(readSize))) == false) {
                return false;
            }

            int sleepTime = ICatEventApi::GetIOReadWriteCount(core_);
            if (sleepTime > IO_SLEEP_NUM) {
                std::this_thread::sleep_for(std::chrono::microseconds((sleepTime - IO_SLEEP_NUM) * IO_DELAY_TIME));
            }
        }
    }

    return true;
}

void HRawStringProtocol::ParseLog(const std::string &log)
{
#ifdef DEBUG_LOG_SIGNAL
    ICatEventApi::ForwardSignal(core_, this, DebugLog, log);
#endif
}

bool HRawStringProtocol::DelaySend()
{
    std::list<delaySend> sendList;
    sendList.swap(delaySend_);

    for (auto &&i : sendList) {
        if (Dispose(i.cmd, i.data, i.userData) == false) {
            return false;
        }
    }

    return true;
}
