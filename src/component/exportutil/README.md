# ExportUtil 导出模块

本模块负责将比赛成绩导出为 HTML 或 CSV 格式。

HTML 导出采用 JSON + 模板 方案：C++ 端通过 `buildExportJson()` 构建包含全部比赛数据的 JSON 对象，然后将其嵌入 `export_template.html` 中的 `%%DATA%%` 占位符，由浏览器端 JavaScript 完成页面渲染。这种方式将数据构造与页面展示解耦，C++ 代码只需关注 JSON 结构，无需拼接 HTML 字符串。

以下是 JSON 数据的结构说明。

### 顶级字段

| 字段名        | 类型       | 说明             | 举例                                  |
| ------------- | ---------- | ---------------- | ------------------------------------- |
| `name`        | `string`   | 比赛名称         | `"contest"`                           |
| `version`     | `string`   | LemonLime 版本   | `"Lemonlime Version 0.3.6.1:7545e02"` |
| `task_names`  | `string[]` | 题目名称数组     | `["plus", "minus"]`                   |
| `i18n`        | `object`   | i18n 字典        | 见下节                                |
| `contestants` | `object[]` | 选手成绩数据数组 | 见下节                                |

### i18n

| 字段名        | 举例                                                      |
| ------------- | --------------------------------------------------------- |
| `rank_list`   | `"排名表"`                                                |
| `hint`        | `"点击名字或单题分数跳转到详细信息。使用 LemonLime 评测"`       |
| `rank`        | `"排名"`                                                  |
| `name`        | `"名称"`                                                  |
| `total`       | `"总分"`                                                  |
| `contestant`  | `"选手"`                                                  |
| `task`        | `"试题"`                                                  |
| `testcase`    | `"测试点"`                                                |
| `input`       | `"输入文件"`                                              |
| `result`      | `"测试结果"`                                              |
| `time`        | `"运行用时"`                                              |
| `memory`      | `"内存消耗"`                                              |
| `score`       | `"得分"`                                                  |
| `return_to_top` | `"返回顶部"`                                            |

---

### contestants 数组内对象

| 字段名         | 类型       | 说明                 | 举例            |
| -------------- | ---------- | -------------------- | --------------- |
| `name`         | `string`   | 选手姓名             | `"Alice"`       |
| `rank`         | `number`   | 选手排名             | `1`             |
| `total_score`  | `number`   | 选手总分             | `200`           |
| `total_bg`     | `string`   | 总分单元格背景 (HSL) | `"0, 70%, 90%"` |
| `total_border` | `string`   | 总分单元格边框 (HSL) | `"0, 70%, 50%"` |
| `tasks`        | `object[]` | 题目结果数组         | 见下节          |

---

### tasks 数组内对象

| 字段名              | 类型       | 说明                       | 举例              |
| ------------------- | ---------- | -------------------------- | ----------------- |
| `score`             | `number`   | 该题单项得分               | `100`             |
| `bg`                | `string`   | 该题得分背景 (HSL)         | `"120, 30%, 60%"` |
| `info`              | `string`   | 评测状态/源文件信息       | `"源程序：plus.cpp"` |
| `compile_message`   | `string`   | 编译错误详情（可选）       | `"error: expected ';'"` |
| `details`           | `object[]` | 测试点详情数组             | 见下节            |

---

### details 数组内对象

| 字段名       | 类型     | 说明                                                 | 举例                           |
| ------------ | -------- | ---------------------------------------------------- | ------------------------------ |
| `label`      | `string` | 测试点编号或子任务信息，允许使用 `<br>`              | `"#2<br/>子任务依赖情况:Pure"` |
| `row_span`   | `number` | 合并单元格行数（0 表示该行被合并，1 表示该行不合并） | `1`                            |
| `input`      | `string` | 该测试点输入文件名                                   | `"plus2.in"`                   |
| `result`     | `string` | 评测状态结果文字                                     | `"评测通过"`                   |
| `time`       | `string` | 运行耗时字符串                                       | `"0.005 s"`                    |
| `memory`     | `string` | 内存占用字符串                                       | `"5.34 MiB"`                   |
| `score`      | `number` | 该项得分                                             | `20`                           |
| `full_score` | `number` | 该项满分                                             | `20`                           |
| `bg`      | `string`   | 状态背景 (RGB) | `"rgb(192, 255, 192)"` |
| `info` | `string` | 测评信息（可选） | `在第四行，读取到 123456，但期望 789123` |

---

### 示例 JSON

以下 JSON 可用于调试模板渲染：

```json
{
    "name": "example contest",
    "version": "Lemonlime Version 0.3.6.1:7545e02",
    "i18n": {
        "rank_list": "排名表",
        "hint": "点击名字或单题分数跳转到详细信息。使用 LemonLime 评测",
        "rank": "排名",
        "name": "名称",
        "total": "总分",
        "contestant": "选手",
        "task": "试题",
        "testcase": "测试点",
        "input": "输入文件",
        "result": "测试结果",
        "time": "运行用时",
        "memory": "内存消耗",
        "score": "得分"
    },
    "task_names": ["plus", "minus"],
    "contestants": [
        {
            "name": "Alice",
            "rank": 1,
            "total_score": 150,
            "total_bg": "120, 30%, 90%",
            "total_border": "120, 30%, 50%",
            "tasks": [
                {
                    "score": 100,
                    "bg": "120, 30%, 70%",
                    "info": "源程序：plus.cpp",
                    "details": [
                        { "label": "#1", "row_span": 1, "input": "plus1.in", "result": "评测通过", "time": "0.001 s", "memory": "1.2 MiB", "score": 50, "full_score": 50, "bg": "rgb(192, 255, 192)" },
                        { "label": "#2", "row_span": 1, "input": "plus2.in", "result": "评测通过", "time": "0.002 s", "memory": "1.2 MiB", "score": 50, "full_score": 50, "bg": "rgb(192, 255, 192)" }
                    ]
                },
                {
                    "score": 50,
                    "bg": "120,28.9006%,82.3499%",
                    "info": "源程序：minus.cpp",
                    "details": [
                        { "label": "#1", "row_span": 1, "input": "minus1.in", "result": "评测通过", "time": "0.001 s", "memory": "1.2 MiB", "score": 50, "full_score": 50, "bg": "rgb(192, 255, 192)" },
                        { "label": "#2<br>子任务依赖情况:Pure", "row_span": 2, "input": "minus2.in", "result": "答案错误", "time": "0.001 s", "memory": "2.0 MiB", "score": 0, "full_score": 50, "bg": "rgb(255, 192, 192)", "info": "在第四行，读取到 123456，但期望 789123" },
                        { "label": "", "row_span": 0, "input": "minus3.in", "result": "运行错误", "time": "0.001 s", "memory": "2.0 MiB", "score": 0, "full_score": 50, "bg": "rgb(255, 192, 192)" }
                    ]
                }
            ]
        },
        {
            "name": "Bob",
            "rank": 2,
            "total_score": 0,
            "total_bg": "0, 0%, 90%",
            "total_border": "0, 0%, 70%",
            "tasks": [
                { "score": 0, "bg": "0, 0%, 90%", "info": "未测试" },
                { "score": 0, "bg": "0, 0%, 90%", "info": "未测试" }
            ]
        }
    ]
}
```
