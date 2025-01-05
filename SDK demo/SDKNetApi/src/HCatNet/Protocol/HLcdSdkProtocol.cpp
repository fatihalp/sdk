

#include <Protocol/HLcdSdkProtocol.h>
#include <Protocol/HStruct.h>
#include <Tool/HTool.h>
#include <CatString.h>
#include <HCatTool.h>
#include <HCatTimer.h>
#include <HCatSignalSet.h>


#define PACKAGE_MAX                 (8196)
#define Get_JSON_MAX(value)         (PACKAGE_MAX - value)
#define LCD_VERSION                 (0x01000000)

#define READ_FILE_MAX               (Get_JSON_MAX(sizeof(cat::HHeader)) * 10)


using namespace cat;



HLcdSdkProtocol::HLcdSdkProtocol(const EventCoreWeakPtr &core)
    : IProtocolBase(core)
    , progress_(new HCatTimer())
    , signal_(new HCatSignalSet())
    , prevSendSize_(0)
    , sendfaild_(false)
    , negotiate_(false)
    , processing_(false)
{
    *signal_ += progress_->Timeout.Bind(this, &HLcdSdkProtocol::NotifyProgress);
}

HLcdSdkProtocol::~HLcdSdkProtocol()
{
    // 线程正在发送文件, 使其失败
    sendfaild_ = true;
    while (!threadRun_.expired() && !core_.expired()) {
        sendfaild_ = true;
        std::this_thread::yield();
    }
}

bool HLcdSdkProtocol::Dispose(hint32 cmd, const HCatBuffer &data, const HCatAny &userData)
{
    switch (cmd) {
    case kInit: {
        this->InitData();
        return true;
    } break;
    case kReset: {
        negotiate_ = true;
        return true;
    } break;
    case kSendFaildAsk: {
        this->sendfaild_ = true;
        return true;
    } break;
    case kConnect: {
        this->progress_->Stop();
        this->sendfaild_ = false;
        return this->ParseNegotiate();
    } break;
    case kReadData: {
        return this->ParseReadData(data, userData);
    } break;
    case kHeartBeat: {
        if (negotiate_ == false) {
            return true;
        }
        return this->ParseHeartBeat();
    } break;
    default:
        break;
    }

    if (processing_) {
        ParseLog("Processing..., Data will be sent late");
        delaySend_.emplace_back(delaySend(cmd, data, userData));
        return true;
    }

    switch (cmd) {
    case kSendSDK: {
        return this->ParseSendSDK(cmd, data, userData);
    } break;
    case kSendFile: {
        if (negotiate_ == false) {
            delaySend_.emplace_back(delaySend(cmd, data, userData));
            return true;
        }

        processing_ = SendFileListStart(data, userData);
        return processing_;
    } break;
    default:
        break;
    }

    return false;
}

std::list<HCatBuffer> HLcdSdkProtocol::SplitSdkData(const HCatBuffer &json)
{
    std::list<HCatBuffer> queue;
    std::size_t size = json.Size();
    huint32 index = 0;

    const std::size_t unitMax = Get_JSON_MAX(sizeof(HSDKCmd));
    for (; size > unitMax; size -= unitMax, index += unitMax){
        HSDKCmd sdk;
        sdk.len = static_cast<huint16>(sizeof(HSDKCmd)) + static_cast<huint16>(unitMax);
        sdk.cmd = HCmdType::kLCDMsgAsk;
        sdk.total = json.Size();
        sdk.index = index;

        HCatBuffer data = HTool::IconvStr(sdk.len);
        data.Append(HTool::IconvStr(sdk.cmd));
        data.Append(HTool::IconvStr(sdk.total));
        data.Append(HTool::IconvStr(sdk.index));
        data.Append(HCatBuffer(json.ConstData() + index, unitMax));

        queue.emplace_back(std::move(data));
    }

    if (size > 0){
        HSDKCmd sdk;
        sdk.len = static_cast<huint16>(sizeof(HSDKCmd)) + static_cast<huint16>(size);
        sdk.cmd = HCmdType::kLCDMsgAsk;
        sdk.total = json.Size();
        sdk.index = index;

        HCatBuffer data = HTool::IconvStr(sdk.len);
        data.Append(HTool::IconvStr(sdk.cmd));
        data.Append(HTool::IconvStr(sdk.total));
        data.Append(HTool::IconvStr(sdk.index));
        data.Append(HCatBuffer(json.ConstData() + index, size));

        queue.emplace_back(std::move(data));
    }

    return queue;
}

std::list<HCatBuffer> HLcdSdkProtocol::SplitFileData(const HCatBuffer &fileData)
{
    std::list<HCatBuffer> queue;
    std::size_t size = fileData.Size();
    huint32 index = 0;

    const std::size_t unitMax = Get_JSON_MAX(sizeof(HHeader));
    for (; size > unitMax; size -= unitMax, index += unitMax){
        HHeader sdk;
        sdk.len = static_cast<huint16>(sizeof(HHeader)) + static_cast<huint16>(unitMax);
        sdk.cmd = HCmdType::kFileContentAsk;

        HCatBuffer data = HTool::IconvStr(sdk.len);
        data.Append(HTool::IconvStr(sdk.cmd));
        data.Append(HCatBuffer(fileData.ConstData() + index, unitMax));

        queue.emplace_back(std::move(data));
    }

    if (size > 0){
        HHeader sdk;
        sdk.len = static_cast<huint16>(sizeof(HHeader)) + static_cast<huint16>(size);
        sdk.cmd = HCmdType::kFileContentAsk;

        HCatBuffer data = HTool::IconvStr(sdk.len);
        data.Append(HTool::IconvStr(sdk.cmd));
        data.Append(HCatBuffer(fileData.ConstData() + index, size));

        queue.emplace_back(std::move(data));
    }

    return queue;
}

void HLcdSdkProtocol::InitData()
{
    negotiate_ = false;
    processing_ = false;
    sendfaild_ = false;
    prevSendSize_ = 0;
    sdkBuff_.Clear();
    readBuff_.Clear();
    delaySend_.clear();
    lastFile_.clear();
    progress_->Stop();
}

bool HLcdSdkProtocol::SendData(const HCatBuffer &data, const HCatAny &userData)
{
    SendNotSDKData(data, userData);
    const HSDKCmd *sdk = reinterpret_cast<const HSDKCmd *>(data.ConstData());
    ParseLog(fmt::format("cmd-{}, Send LCD Data[{}], offset[{}], total[{}]", HCmdType::HCmdTypeStr(sdk->cmd), data.Size() - sizeof(HSDKCmd), sdk->index, sdk->total));
    return !sendfaild_;
}

bool HLcdSdkProtocol::SendNotSDKData(const HCatBuffer &data, const HCatAny &userData)
{
    const HHeader *header = reinterpret_cast<const HHeader *>(data.ConstData());
    ParseLog(fmt::format("send-{}", HCmdType::HCmdTypeStr(header->cmd)));
    return SendSourceData(data, userData);
}

bool HLcdSdkProtocol::SendSourceData(const HCatBuffer &data, const HCatAny &userData)
{
    NotifyCmd(kSendData, data, userData);
    return !sendfaild_;
}

void HLcdSdkProtocol::NotifyReadyData(const HCatBuffer &data, const HCatAny &userData)
{
    NotifyCmd(kReadyRead, data, userData);
}

void HLcdSdkProtocol::NotifySendFileStatus(const HCatBuffer &data, const HCatAny &userData)
{
    NotifyCmd(kUploadFileProgress, data, userData);
}

void HLcdSdkProtocol::NotifySendFlowStatus(int status, const HCatAny &userData)
{
    if (!flow_ || flow_->userData.IsType<HFlowFileList>() == false) {
        return ;
    }

    HFlowFileList &flow = flow_->userData.Cast<HFlowFileList>();
    HCatBuffer data = fmt::format("{};;{};;{}", flow.SendFlow(flow_->send), flow.FlowTotal(), status);
    NotifyCmd(kUploadFlowProgress, data, userData);
}

void HLcdSdkProtocol::NotifyReset()
{
    NotifyCmd(kSendFaildAnswer, HCatBuffer(), HCatAny());
}

bool HLcdSdkProtocol::ParseReadData(const HCatBuffer &data, const HCatAny &userData)
{
    readBuff_.Append(data);
    HHeader header(readBuff_.GetString(sizeof(HHeader)));

    if (readBuff_.Size() < sizeof(HHeader)) {
        return true;
    }

    if (header.len > readBuff_.Size()) {
        return true;
    }

    // 切包
    HCatBuffer buff = readBuff_.Mid(0, header.len);
    readBuff_.Remove(0, header.len);

    // 输出日志信息
    ParseLog(fmt::format("read-{}", HCmdType::HCmdTypeStr(header.cmd)));

    bool result = true;
    switch (header.cmd) {
    case HCmdType::kTcpHeartbeatAsk: {
        result = this->ParseHeartBeat(false);
    } break;
    case HCmdType::kTcpHeartbeatAnswer:
        break;
    case HCmdType::kLCDServiceAnswer: {
        result = this->ParseLCDServiceAnswer(buff);
    } break;
    case HCmdType::kLCDMsgAnswer: {
        result = this->ParseLCDMsgAnswer(buff);
    } break;
    case HCmdType::kErrorAnswer: {
        const HErrorStatus *status = reinterpret_cast<const HErrorStatus *>(buff.ConstData());
        processing_ = false;
        ParseLog(fmt::format("read error code[{}], status[{}]", HCmdType::HCmdTypeStr(status->head.cmd), HErrorCode::HErrorCodeStr(status->status)));
        NotifyReadyData(HCatBuffer(),  HReadProtocolData(HReadProtocolData::kErrorStatus, status->status));
    } break;
    case HCmdType::kFileListStartAnswer: {
        result = this->ParseFileListStartAnswer(buff, userData);
    } break;
    case HCmdType::kFileListEndAnswer: {
        hint64 fileSize = GetFileSize(lastFile_);
        this->NotifySendFileStatus(fmt::format("{};;{};;{};;2", lastFile_, fileSize, fileSize));
        this->NotifySendFlowStatus(2);
        processing_ = false;
        result = this->DelaySend();
    } break;
    case HCmdType::kFileStartAnswer: {
        result = this->ParseFileStartAnswer(data);
    } break;
    case HCmdType::kFileContentAnswer:
        break;
    case HCmdType::kFileEndAnswer: {
        if (buff.Size() < sizeof(HErrorStatus)) {
            ParseLog(fmt::format("Read size error. len[{}]", buff.Size()));
            processing_ = false;
            return false;
        }

        const HErrorStatus *status = reinterpret_cast<const HErrorStatus *>(buff.ConstData());
        if (status->status != 0) {
            ParseLog(fmt::format("read error code[{}], status[{}]", HCmdType::HCmdTypeStr(status->head.cmd), HErrorCode::HErrorCodeStr(status->status)));
            NotifyReadyData(HCatBuffer(),  HReadProtocolData(HReadProtocolData::kErrorStatus, status->status));
        }

        if (sendFileList_.fileList.empty() == false) {
            lastFile_ = sendFileList_.fileList.front().filePath;
            sendFileList_.fileList.pop_front();
            this->NotifyProgress();
        }

        result = ParseSendFileAnswer(userData);
    } break;
    default:
        ParseLog(fmt::format("Unresolved-{}", HCmdType::HCmdTypeStr(header.cmd)));
        NotifyCmd(kReadyRawRead, buff, userData);
        break;
    }

    if (result && readBuff_.Empty() == false) {
        return this->ParseReadData(HCatBuffer(), userData);
    }

    return result;
}

bool HLcdSdkProtocol::ParseNegotiate()
{
    HTcpVersion version;
    version.len = sizeof(HTcpVersion);
    version.cmd = HCmdType::kLCDServiceAsk;
    version.version = LCD_VERSION;

    HCatBuffer data = HTool::IconvStr(version.len);
    data.Append(HTool::IconvStr(version.cmd));
    data.Append(HTool::IconvStr(version.version));

    if (SendNotSDKData(data) == false) {
        this->NotifyReset();
        ParseLog("Negotiate faild.");
        return false;
    }

    return true;
}

bool HLcdSdkProtocol::ParseLCDServiceAnswer(const HCatBuffer &data)
{
    if (data.Size() < sizeof(HTcpVersion)) {
        return false;
    }

    const HTcpVersion *version = reinterpret_cast<const HTcpVersion *>(data.ConstData());
    ParseLog(fmt::format("Lcd cmd[{}], version[{}]", HCmdType::HCmdTypeStr(version->cmd), version->version));
    if (version->version < LCD_VERSION) {
        return false;
    }

    NotifyCmd(kNotofyConnect, std::string(), HCatAny());

    // 完成协商, 开始进行延迟发送
    negotiate_ = true;
    return DelaySend();
}

bool HLcdSdkProtocol::ParseLCDMsgAnswer(HCatBuffer &data)
{
    HSDKCmd sdkCmd(data.GetString(sizeof(HSDKCmd)));
    data.Remove(0, sizeof(HSDKCmd));

    if (data.Empty() == false) {
        sdkBuff_.Append(data);
        if (sdkCmd.total <= sdkBuff_.Size()) {
            this->NotifyReadyData(sdkBuff_);
            sdkBuff_.Clear();
        }
    }

    return true;
}

bool HLcdSdkProtocol::ParseHeartBeat(bool ask)
{
    HHeader heartbeat(sizeof(HHeader), ask ? HCmdType::kTcpHeartbeatAsk : HCmdType::kTcpHeartbeatAnswer);
    HCatBuffer data;
    data.Append(HTool::IconvStr(heartbeat.len));
    data.Append(HTool::IconvStr(heartbeat.cmd));
    return SendNotSDKData(data);
}

bool HLcdSdkProtocol::ParseSendSDK(hint32 cmd, const HCatBuffer &data, const HCatAny &userData)
{
    // 还未协商完, 完成后会自动补发
    if (negotiate_ == false) {
        delaySend_.emplace_back(delaySend(cmd, data, userData));
        return true;
    }

    if (data.Empty()) {
        return true;
    }

    auto queue = SplitSdkData(data);
    for (auto &i : queue) {
        if (SendData(i, userData) == false) {
            this->NotifyReset();
            return false;
        }
    }

    return true;
}

bool HLcdSdkProtocol::SendFileListStart(const HCatBuffer &data, const HCatAny &userData)
{
    (void)data;
    if (userData.IsType<HSendProtocolFileInfo>() == false) {
        ParseLog(fmt::format("Send file list type error[{}]", userData.Type().name()));
        return false;
    }

    sendFileList_ = userData.Cast<HSendProtocolFileInfo>();
    if (sendFileList_.fileList.empty()) {
        return false;
    }

    std::string guid;
    if (sendFileList_.userData.IsType<std::string>()) {
        guid = sendFileList_.userData.Cast<std::string>();
    }

    if (guid.empty()) {
        guid = HTool::Uuid();
    }

    std::vector<char> fileListStart(sizeof(HFileListStartAsk) + guid.size() + sizeof(char), 0);
    HFileListStartAsk *ask = reinterpret_cast<HFileListStartAsk *>(fileListStart.data());
    ask->len = static_cast<huint16>(fileListStart.size());
    ask->cmd = HCmdType::kFileListStartAsk;
    ask->fileCount = static_cast<huint16>(sendFileList_.fileList.size());
    ask->AllFileSize = 0;

    flow_.reset(new HFlowCount());
    HFlowFileList flowList;
    for (const auto &i : sendFileList_.fileList) {
        HFlowFile flow(i.filePath, GetFileSize(i.filePath));
        flow.SetFlowSize(sizeof(HHeader), Get_JSON_MAX(sizeof(HHeader)), READ_FILE_MAX);
        ask->AllFileSize += flow.fileSize;
        flowList.flowList.emplace_back(std::move(flow));
    }
    flowList.AdjustFlowList().AdjustTotal();
    flowList.headSize += fileListStart.size();
    flow_->userData = flowList;
    prevSendSize_ = 0;
    NotifyCmd(kSetFlowMonitor, HCatBuffer(), flow_);
    memcpy(ask->sessionID, guid.data(), guid.size());

    return SendNotSDKData(std::string(fileListStart.data(), fileListStart.size()));
}

bool HLcdSdkProtocol::ParseFileListStartAnswer(const HCatBuffer &data, const HCatAny &userData)
{
    const HFileListStartAnswer *status = reinterpret_cast<const HFileListStartAnswer *>(data.ConstData());
    if (status->status != 0) {
        ParseLog(fmt::format("read error code[{}], status[{}]", HCmdType::HCmdTypeStr(status->cmd), HErrorCode::HErrorCodeStr(status->status)));
        NotifyReadyData(HCatBuffer(), HReadProtocolData(HReadProtocolData::kErrorStatus, status->status));
        processing_ = false;
        return this->DelaySend();
    }

    return this->ParseSendFileAnswer(userData);
}

bool HLcdSdkProtocol::ParseSendFileAnswer(const HCatAny &userData)
{
    progress_->Start(core_, PROGRESS_TIMEOUT);
    if (sendFileList_.fileList.empty()) {
        return this->SendFileListEnd();
    }

    survivalTest_typePtr run(new survivalTest_type());
    threadRun_ = run;
    ICatEventApi::AddThread(core_, run, this, &HLcdSdkProtocol::SendFileStart, sendFileList_.fileList.front(), userData);
    return true;
}

bool HLcdSdkProtocol::SendFileListEnd()
{
    std::vector<char> data(sizeof(HHeader), 0);
    HHeader *ask = reinterpret_cast<HHeader *>(data.data());
    ask->len = static_cast<huint16>(data.size());
    ask->cmd = HCmdType::kFileListEndAsk;
    if (flow_ && flow_->userData.IsType<HFlowFileList>()) {
        flow_->userData.Cast<HFlowFileList>().headSize += data.size();
    }

    return SendNotSDKData(HCatBuffer(data.data(), data.size()));
}

bool HLcdSdkProtocol::SendFileStart(const HSendFile &sendFile, const HCatAny &userData)
{
    if (sendfaild_) {
        return false;
    }

    std::string filePath = sendFile.filePath;
    std::string md5 = sendFile.md5;
    if (md5.empty()) {
        md5 = HTool::MD5_U8(filePath, &sendfaild_);
        if (sendfaild_) {
            return false;
        }
    }

    if (md5.empty()) {
        ParseLog(fmt::format("File cannot be opened. file[{}]", filePath));
        return false;
    }

    std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
    if (sendFile.fileName.empty() == false) {
        fileName = sendFile.fileName;
    }
    std::string fileAsk(fileName.size() + sizeof(HFileStartAsk) - sizeof(hint8), '\0');

    HFileStartAsk *ask = reinterpret_cast<HFileStartAsk *>(&fileAsk.front());
    ask->len = static_cast<huint16>(fileAsk.size());
    ask->cmd = HCmdType::kFileStartAsk;
    ask->size = GetFileSize(filePath);
    strcpy(ask->md5, md5.data());
    strcpy(ask->name, fileName.data());
    ask->type = sendFile.type;

    if (flow_ && flow_->userData.IsType<HFlowFileList>()) {
        flow_->userData.Cast<HFlowFileList>().headSize += fileAsk.size();
    }

    if (SendNotSDKData(fileAsk, userData) == false) {
        this->NotifyReset();
        return false;
    }

    return true;
}

bool HLcdSdkProtocol::ParseFileStartAnswer(const HCatBuffer &data)
{
    do {
        if (sendFileList_.fileList.empty()) {
            ParseLog("Sending file name is empty.");
            return false;
        }

        const HFileStartAnswer *answer = reinterpret_cast<const HFileStartAnswer *>(data.ConstData());
        if (answer->status != 0) {
            ParseLog(fmt::format("read status error[{}]", answer->status));
            processing_ = false;
            return DelaySend();
        }

        lastFile_ = sendFileList_.fileList.front().filePath;
        threadSendFilePtr callBack(new FileCallBack);
        callBack->sendFile.open(IconvFilePath(lastFile_), std::ios::in | std::ios::binary);
        if (callBack->sendFile.is_open() == false) {
            ParseLog(fmt::format("open file[{}] faild. error[{}]", lastFile_, HCatTool::GetError()));
            break;
        }

        callBack->sendFile.seekg(0, std::ios::end);
        hint64 fileSize = callBack->sendFile.tellg();
        callBack->sendFile.seekg(0, std::ios::beg);
        hint64 sendSize = answer->existSize;
        if (flow_ && flow_->userData.IsType<HFlowFileList>()) {
            flow_->userData.Cast<HFlowFileList>().AddAdjustFlow(sendSize, sizeof(HHeader), Get_JSON_MAX(sizeof(HHeader)), READ_FILE_MAX);
        }

        ParseLog(fmt::format("File exists size[{}]", sendSize));
        if (fileSize == sendSize) {
            ParseLog(fmt::format("File already exists. file[{}]", lastFile_));
            NotifySendFileStatus(fmt::format("{};;{};;{};;0", lastFile_, sendSize, fileSize));
            this->NotifySendFlowStatus(0);
            break;
        }

        callBack->GetSendData(sendSize);
        callBack->filePath = lastFile_;
        callBack->fileSize = fileSize;
        callBack->sendSize = sendSize;
        if (callBack->sendData.Empty()) {
            break;
        }

        if (SendNotSDKData(callBack->sendData) == false) {
            ParseLog(fmt::format("File sending failed. file[{}]. send size[{}]", lastFile_, sendSize));
            return false;
        }

        callBack->sendSize += callBack->readSize;
        survivalTest_typePtr run(new survivalTest_type());
        threadRun_ = run;
        ICatEventApi::AddThread(core_, run, this, &HLcdSdkProtocol::ThreadSendFile, HCatAny(callBack));
        return true;

    } while (false);

    return this->SendFileEnd();
}

bool HLcdSdkProtocol::ThreadSendFile(const HCatAny &userData)
{
    if (sendfaild_) {
        return false;
    }

    if (userData.IsType<threadSendFilePtr>() == false) {
        SendFileEnd();
        return false;
    }

    threadSendFilePtr callBack = userData.Cast<threadSendFilePtr>();
    for (;;) {
        if (sendfaild_) {
            return false;
        }

        callBack->GetSendData();
        if (callBack->sendData.Empty()) {
            break;
        }

        if (SendSourceData(callBack->sendData) == false) {
            ICatEventApi::Forward(core_, true, this, &HLcdSdkProtocol::NotifySendFileStatus, fmt::format("{};;{};;{};;1", callBack->filePath, callBack->sendSize, callBack->fileSize), userData);
            ICatEventApi::Forward(core_, true, this, &HLcdSdkProtocol::NotifySendFlowStatus, 1, userData);
            return false;
        }

        callBack->sendSize += callBack->readSize;
        int sleepTime = ICatEventApi::GetIOReadWriteCount(core_);
        if (sleepTime > IO_SLEEP_NUM) {
            std::this_thread::sleep_for(std::chrono::microseconds((sleepTime - IO_SLEEP_NUM) * IO_DELAY_TIME));
        }
    }

    return SendFileEnd();
}

bool HLcdSdkProtocol::SendFileEnd()
{
    char sendEnd[sizeof(HHeader)];
    HHeader *ask = reinterpret_cast<HHeader *>(sendEnd);
    ask->len = sizeof(HHeader);
    ask->cmd = HCmdType::kFileEndAsk;

    if (flow_ && flow_->userData.IsType<HFlowFileList>()) {
        flow_->userData.Cast<HFlowFileList>().headSize += ask->len;
    }
    if (SendNotSDKData(HCatBuffer(sendEnd, sizeof(HHeader))) == false) {
        this->NotifyReset();
        ParseLog("Send file End request faild");
        return false;
    }
    return true;
}

void HLcdSdkProtocol::NotifyProgress()
{
    if (processing_ == false || sendfaild_) {
        progress_->Stop();
        return ;
    }

    auto flowPtr = flow_;
    if (!flowPtr || flowPtr->userData.IsType<HFlowFileList>() == false) {
        progress_->Stop();
        return ;
    }

    if (prevSendSize_ == flowPtr->send) {
        return ;
    }

    const HFlowFileList &flow = flowPtr->userData.Cast<HFlowFileList>();
    auto currFile = flow.GetFilePack(flowPtr->send);
    if (prevSendSize_ == 0) {
        prevSendSize_ = flowPtr->send;
    }

    if (flow.IsFinish(flowPtr->send) == false) {
        auto prevFile = flow.GetFilePack(prevSendSize_);
        if (currFile != prevFile) {
            NotifySendFileStatus(fmt::format("{};;{};;{};;3", prevFile.path, prevFile.size, prevFile.size));
        }
    }
    prevSendSize_ = flowPtr->send;
    NotifySendFileStatus(fmt::format("{};;{};;{};;0", currFile.path, currFile.send, currFile.size));
    this->NotifySendFlowStatus(0);

    if (flow.IsFinish(flowPtr->send)) {
        progress_->Stop();
        return ;
    }
}

bool HLcdSdkProtocol::DelaySend()
{
    std::list<delaySend> sendList;
    sendList.swap(delaySend_);
    while (sendList.empty() == false) {
        if (processing_) {
            delaySend_.splice(delaySend_.begin(), std::move(sendList));
            break;
        }

        delaySend i = std::move(sendList.front());
        sendList.pop_front();

        if (Dispose(i.cmd, i.data, i.userData) == false) {
            ParseLog("Delay Send Faild.");
            delaySend_.clear();
            return false;
        }
    }

    return true;
}

hint64 HLcdSdkProtocol::GetFileSize(const std::string &filePath)
{
    std::fstream sendFile;
    sendFile.open(IconvFilePath(filePath), std::ios::in | std::ios::binary);
    if (sendFile.is_open() == false) {
        ParseLog(fmt::format("open file[{}] faild", filePath));
        return 0;
    }

    sendFile.seekg(0, std::ios::end);
    hint64 fileSize = sendFile.tellg();
    sendFile.close();
    return fileSize;
}

#ifdef H_WIN
std::wstring HLcdSdkProtocol::IconvFilePath(const std::string &filePath) const
#else
std::string HLcdSdkProtocol::IconvFilePath(const std::string &filePath) const
#endif
{
#ifdef H_WIN
    return cat::HCatTool::UTF8ToUnicode(filePath);
#else
    return filePath;
#endif
}

void HLcdSdkProtocol::ParseLog(const std::string &log)
{
#ifdef DEBUG_LOG_SIGNAL
    ICatEventApi::ForwardSignal(core_, this, DebugLog, log);
#endif
}

bool HLcdSdkProtocol::FileCallBack::GetSendData(hint64 offset)
{
    sendData.Clear();

    if (sendFile.is_open() == false) {
        return false;
    }

    if (offset != -1) {
        sendFile.seekg(offset);
    }

    std::vector<char> data(READ_FILE_MAX);
    sendFile.read(data.data(), data.size());
    readSize = sendFile.gcount();
    if (readSize <= 0) {
        sendFile.close();
        return false;
    }

    auto splitData = SplitFileData(HCatBuffer(data.data(), static_cast<std::size_t>(readSize)));
    for (auto &&i: splitData) {
        sendData.Append(std::move(i));
    }
    return true;
}
