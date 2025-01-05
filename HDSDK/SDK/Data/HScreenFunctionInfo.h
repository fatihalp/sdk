

#ifndef __HSCREENFUNCTIONINFO_H__
#define __HSCREENFUNCTIONINFO_H__


#include <HCatBuffer.h>
#include <HXml.h>
#include <cstddef>
#include <vector>


namespace sdk
{


///< 开关机
struct SwitchTimeInfo
{
    struct ployItem {
        bool enable;            ///< 该项是否使能, 取值范围{"true"(使能), "false"(不使能)}
        cat::HCatBuffer start;  ///< 开屏时刻, 格式hh:mm:ss
        cat::HCatBuffer end;    ///< 关屏时刻, 格式hh:mm:ss
    };

    struct weekItem {
        cat::HCatBuffer start;  ///< 格式hh:mm:ss
        cat::HCatBuffer end;    ///< 格式hh:mm:ss
    };

    struct weekPloyItem {
        int week;                       ///< 星期数[0-6]
        bool openAllDay;                ///< 全天开机
        std::vector<weekItem> ploys;    ///< 时间项
        weekPloyItem()
            : week(0)
            , openAllDay(true)
        {}
    };

    bool open;                              ///< 当前屏幕状态
    bool ployEnable;                        ///< 是否启用定时开关屏
    int mode;                               ///< 开关屏模式 0: 每天, 1:每星期
    std::vector<ployItem> ploys;            ///< 开关屏时间段列表
    std::vector<weekPloyItem> weekPloys;    ///< 按星期开关屏时间表

    void InitWeekPloys();
    static std::size_t GetWeekMax() { return 7; }
    weekPloyItem &GetWeekItem(int week);

    static bool MatchGet(const cat::HCatBuffer &item) { return item == "GetSwitchTime"; }
    static bool MatchSet(const cat::HCatBuffer &item) { return item == "SetSwitchTime"; }
    static cat::HCatBuffer GetMethod() { return "GetSwitchTime"; }
    static cat::HCatBuffer SetMethod() { return "SetSwitchTime"; }
};
void from_xml(const HXml &xml, SwitchTimeInfo &node);
void to_xml(HXml &xml, const SwitchTimeInfo &node);


}



#endif // HSCREENFUNCTIONINFO_H
