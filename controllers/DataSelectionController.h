#include <string>
#include <vector>
#include <map>

class DataSelectionView; // 前向声明

class DataSelectionController {
public:
    static DataSelectionController& getInstance();
    ~DataSelectionController();

    void init(DataSelectionView* view);
    DataSelectionView* getView() const;
    std::vector<std::vector<std::string>> getRelatedTasks(int radiationId);
    // 删除选中的数据项
    void deleteSelectedItems(DataSelectionView* view);
    // 向数据库录入数据
    bool importData(DataSelectionView* view, bool isSingle, const std::vector<std::string>& values,
                   int deviceId, int radiationId, const std::string& techSystem);
    // 显示任务详情
    std::map<std::string, std::string> showTaskDetails(int taskId, const std::string& taskType);

private:
    DataSelectionController();
    DataSelectionController(const DataSelectionController&) = delete;
    DataSelectionController& operator=(const DataSelectionController&) = delete;

    DataSelectionView* m_view;
    std::vector<std::string> m_dataItems;
};
