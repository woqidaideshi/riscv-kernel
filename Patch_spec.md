riscv-kernel 补丁合入规范
=========================

修订记录

| 日期      | 修订版本 | 修改章节 | 修改描述 | 作者         |
| --------- | -------- | -------- | -------- | ------------ |
| 2024.3.28 | 1.0      |          | 初稿     | 邢明政       |

1 总览
----------------------------

本规范内容按照补丁类型分类，可通过下面链接快速访问。

注意本规范在持续完善中，如果发现有任何问题，欢迎提交 issues，或者直接提交 PR。

1. [来自开源社区的 SoC 支持补丁](#21)
2. [处理合并冲突的补丁](#22)
3. [新特性开发和漏洞修复补丁](#23)
4. [来自主线Linux内核的补丁](#24)

### 1.1 为什么要有补丁合入规范

riscv-kernel 积极合入新的 RISC-V SoC 支持补丁，打造支持多 RISC-V 硬件平台的同源内核。因此会不断有来自芯片厂商、SoC社区等开源补丁合入。这些补丁格式的统一将为后期的内核维护工作带来便利。

### 1.2 与openEuler内核补丁提交规范的关系

riscv-kernel 按照与 [openeuler/kernel](https://gitee.com/openeuler/kernel) 代码同源策略管理，因此本仓库补丁需首先遵守 [openEuler内核补丁提交规范](https://gitee.com/openeuler/community/blob/master/sig/Kernel/%E8%A1%A5%E4%B8%81%E6%8F%90%E4%BA%A4%E8%A7%84%E8%8C%83.md)，可先阅读该文档。其中规定了统一的补丁格式，以及编码风格。本规范在上述规范基础上，针对 riscv-kernel 的应用场景和补丁类型进行了细化，并提供了相应的参考示例，目的是为了简化贡献者的补丁提交工作。

2 补丁类型及相应规范
----------------------------

<h3 id="21">2.1 来自开源社区的 SoC 支持补丁</h3>

riscv-kernel 在合入新的 RISC-V SoC 支持补丁过程中，会有来自对应的厂商内核仓库、开源社区等补丁合入。这些补丁格式统一规范如下：

#### 格式定义

```
$SoC-name: $commit-title

community inclusion        [M]
category: feature          [M]
bugzilla: $bug-url         [M]
CVE: $cve-id               [O]
Reference: $refer-url      [O]

--------------------------------

original commitlog
...
[additional changelog]                        [O]
Signed-off-by:$yourname <$yourname@xxx.com>   [M]
```

[M] 代表“强制” [O] 代表“可选”

#### 具体说明

- $SoC-name

该补丁用于支持的 SoC 名称，如 sg2042, th1520 等。

- $commit-title

原补丁的 commit 标题，建议保留原标题的关键信息，优化格式，如删除重复的 SoC 信息、riscv前缀等。

- community inclusion

来自开源社区的 SoC 支持补丁统一使用 `community inclusion`。

- category: feature

来自开源社区的 SoC 支持补丁统一使用 `feature` 类别。

- $bug-url

每个补丁都需要有对应的 bugzilla/issue 相关联。通常是新增特性的需求issue、缺陷报告issue等。对应的补丁或补丁集合入后会关闭相关联的 issue，作为便于后期追踪的信息归档。这种方式将方便我们进行任务管理和进展跟踪，使得内核维护过程清晰可见。补丁合入时，关联的 issue 中需要更新以下内容：

1. 准确描述该任务的目标/要修复的缺陷
2. 对应补丁的测试过程和验证结果
3. 如果涉及内核 config 修改，需要标出具体变化

- $cve-id

如果补丁是修复 CVE 的，那么我们需要标注如 `CVE-2020-1234` 的 cve id，否则请删除此标签。

- $refer-url

提供补丁或补丁集完整的 URL 参考链接。建议链接指向已归档的代码仓库，保持链接长期有效。

- original commitlog

一般为原始的提交日志。

- additional changelog

附加变更日志应至少包括以下内容之一：

1、为什么要合这个补丁 2、这个补丁解决了产品中的哪些实际问题 3、我们如何重现此 bug 或如何测试 4、其他有助于理解此补丁或问题的有用信息

详细信息对于将补丁移植到其他分支或内核重大升级非常有用。 如果您愿意，您也可以在 bugzilla/issue 中提供更多的细节。

- Signed-off-by

在保留原始签名信息的基础上，您也需要在最后加上您的签名。使用您的真实姓名，请勿使用化名或匿名贡献。

#### 示例

```
sg2042: driver: pcie: Add sophgo sg2042 soc support

community inclusion
category: feature
bugzilla: https://gitee.com/openeuler/riscv-kernel/issues/I9DRVT
Reference: https://github.com/xmzzz/linux-riscv/commit/b3ccc12920772a10791da1b32422d2242c8b7d79

--------------------------------

Signed-off-by: Xiaoguang Xing <xiaoguang.xing@sophgo.com>
Signed-off-by: Mingzheng Xing <xingmingzheng@iscas.ac.cn>
```

<h3 id="22">2.2 处理合并冲突的补丁</h3>

添加新的 SoC 支持，由于部分代码尚未合入主线，可能会出现合并冲突。每个冲突需要关联一个对应的 issue 进行追踪，在 issue 中详细描述冲突的相关信息，并及时跟踪。解决冲突的补丁格式需符合本小节规范。

如果需要回退某个 commit，需使用 `git revert` 提供相应的信息描述，并同样按照本小节规范添加补丁头。

#### 格式定义

```
riscv: $commit-title

$inclusion-tags         [M]
category: conflict      [M]
bugzilla: $bug-url      [M]
CVE: $cve-id            [O]
Reference: $refer-url   [O]

--------------------------------

commitlog               [M]

Signed-off-by:$yourname <$yourname@xxx.com>   [M]
```

#### 具体说明

- $commit-title

准确描述该提交的标题信息

- $inclusion-tags

如果是涉及驱动部分的代码补丁，建议统一为 `driver inclusion`；除驱动相关补丁外建议统一为`riscv inclusion`。

- category: conflict

处理合并冲突的补丁统一使用 `conflict` 类别。

- $bug-url

每个补丁都需要有对应的 bugzilla/issue 相关联。通常是新增特性的需求issue、缺陷报告issue等。对应的补丁或补丁集合入后会关闭相关联的 issue，作为便于后期追踪的信息归档。这种方式将方便我们进行任务管理和进展跟踪，使得内核维护过程清晰可见。补丁合入时，关联的 issue 中需要更新以下内容：

1. 准确描述该任务的目标/要修复的缺陷
2. 对应补丁的测试过程和验证结果
3. 如果涉及内核config修改，需要标出具体变化

- $cve-id

如果补丁是修复 CVE 的，那么我们需要标注如 `CVE-2020-1234` 的 cve id，否则请删除此标签。

- $refer-url

可选，可提供补丁相关的 URL 参考链接。如果补丁来自上游社区讨论，可提供相应的邮件列表链接。

- commitlog

详细描述补丁所作的修改，相关规范请参阅 [描述你的改动——Linux内核文档](https://www.kernel.org/doc/html/latest/translations/zh_CN/process/submitting-patches.html#zh-describe-changes)。

- Signed-off-by

需要对您的补丁加上签名，使用您的真实姓名，请勿使用化名或匿名贡献。

#### 示例

```
riscv: thead: Use the wback_inv instead of wback_only

riscv inclusion
category: conflict
bugzilla: https://gitee.com/openeuler/riscv-kernel/issues/I99B44
Reference: https://lore.kernel.org/linux-riscv/ZQBDFa0fGNiaqAgh@gmail.com

--------------------------------

Referring to the existing discussions on the mailing list [1], T-HEAD
processors would prioritize using an invalid cacheline instead of evicting
an existing cacheline. When we do dcache clean, the following operations
are to let other interconnect masters read. So, keeping wback_inv for
T-HEAD processors is the best choice.

Link: https://lore.kernel.org/linux-riscv/ZQBDFa0fGNiaqAgh@gmail.com [1]
Signed-off-by: Mingzheng Xing <xingmingzheng@iscas.ac.cn>
```

<h3 id="23">2.3 新特性开发和漏洞修复补丁</h3>

基于 riscv-kernel 仓进行的新特性开发补丁，或者 bugfix，请参考以下规范。

#### 格式定义

```
riscv: $commit-title

$inclusion-tags            [M]
category: $category        [M]
bugzilla: $bug-url         [M]
CVE: $cve-id               [O]

--------------------------------

commitlog                  [M]

Signed-off-by:$yourname <$yourname@xxx.com>   [M]
```

[M] 代表“强制” [O] 代表“可选”

#### 具体说明

- $inclusion-tags

如果是涉及驱动部分的代码补丁，建议统一为 `driver inclusion`；除驱动相关补丁外建议统一为`riscv inclusion`。

- $category

可以是：cleanup, bugfix, performance, feature, doc, config, other...

- $bug-url

每个补丁都需要有对应的 bugzilla/issue 相关联。通常是新增特性的需求issue、缺陷报告issue等。对应的补丁或补丁集合入后会关闭相关联的 issue，作为便于后期追踪的信息归档。这种方式将方便我们进行任务管理和进展跟踪，使得内核维护过程清晰可见。补丁合入时，关联的 issue 中需要更新以下内容：

1. 准确描述该任务的目标/要修复的缺陷
2. 对应补丁的测试过程和验证结果
3. 如果涉及内核config修改，需要标出具体变化

- $cve-id

如果补丁是修复 CVE 的，那么我们需要标注如 `CVE-2020-1234` 的 cve id，否则请删除此标签。

- commitlog

详细描述补丁所作的修改，相关规范请参阅 [描述你的改动——Linux内核文档](https://www.kernel.org/doc/html/latest/translations/zh_CN/process/submitting-patches.html#zh-describe-changes)。

- Signed-off-by

需要对您的补丁加上签名，使用您的真实姓名，请勿使用化名或匿名贡献。

#### 示例

```
riscv: config: Enable sg2042 support

riscv inclusion
category: config
bugzilla: https://gitee.com/openeuler/riscv-kernel/issues/I9DRVT

--------------------------------

Based on the current openeuler_defconfig for riscv, use the following
commands to generate the new openeuler_defconfig:

cp arch/riscv/configs/openeuler_defconfig .config
cat << EOF >> .config
CONFIG_ARCH_SOPHGO=y
CONFIG_MMC_SDHCI_SOPHGO=y
CONFIG_PCIE_CADENCE_SOPHGO=y
CONFIG_RISCV_ISA_V=n
EOF
make save_oedefconfig
make update_oedefconfig

Build and boot testing passed.

Signed-off-by: Mingzheng Xing <xingmingzheng@iscas.ac.cn>
```

<h3 id="24">2.4 来自主线内核的补丁</h3>

如果是来自主线 Linux kernel 社区的补丁，请参考以下规范。

#### 格式定义

```
$commit-title

$inclusion-tags         [M]
from $version           [M]
commit $id              [M]
category: $category     [M]
bugzilla: $bug-url      [M]
CVE: $cve-id            [O]
Reference: $refer-url   [O]

--------------------------------

original commitlog
...
[additional changelog]                        [O]
Signed-off-by:$yourname <$yourname@xxx.com>   [M]
```

[M] 代表“强制” [O] 代表“可选”

#### 具体说明

- $inclusion-tags

1. 如果补丁来自主线内核，使用 `mainline inclusion`；
2. 如果补丁来自 Linux 稳定分支，使用 `stable inclusion`；
3. 如果补丁来自其他发行版，使用 `dist inclusion`。

- $version

1. 如果是 mainline inclusion，格式为 `mainline-v6.7-rc1`等；
2. 如果是 stable inclusion，格式为 `stable-v5.10.15`等；
3. 如果是 dist inclusion，请提供正确的版本描述。

- $id

1. 如果是 `mainline inclusion` 或 `stable inclusion`，需提供完整的 commit id；
2. `dist inclusion` 类型可选提供，但需要在 `Reference: $refer-url` 处提供URL参考链接。

- category: $category

可以是：cleanup, bugfix, performance, feature, doc, config, other...

- $bug-url

每个补丁都需要有对应的 bugzilla/issue 相关联。通常是新增特性的需求issue、缺陷报告issue等。对应的补丁或补丁集合入后会关闭相关联的 issue，作为便于后期追踪的信息归档。这种方式将方便我们进行任务管理和进展跟踪，使得内核维护过程清晰可见。补丁合入时，关联的 issue 中需要更新以下内容：

1. 准确描述该任务的目标/要修复的缺陷
2. 对应补丁的测试过程和验证结果
3. 如果涉及内核config修改，需要标出具体变化

- $cve-id

如果补丁是修复 CVE 的，那么我们需要标注如 `CVE-2020-1234` 的 cve id，否则请删除此标签。

- $refer-url

如果补丁来自其他发行版，需要提供补丁或补丁集完整的 URL 参考链接。

- original commitlog

一般为原始的提交日志。

- additional changelog

附加变更日志应至少包括以下内容之一：

1、为什么要合这个补丁 2、这个补丁解决了产品中的哪些实际问题 3、我们如何重现此bug或如何测试 4、其他有助于理解此补丁或问题的有用信息

详细信息对于将补丁移植到其他分支或内核重大升级非常有用。 如果您愿意，您也可以在 bugzilla/issue 中提供更多的细节。

- Signed-off-by

在保留原始签名信息的基础上，您也需要在最后加上您的签名。使用您的真实姓名，请勿使用化名或匿名贡献。

#### 示例

```
riscv: mm: update T-Head memory type definitions

mainline inclusion
from mainline-v6.7-rc1
commit dbfbda3bd6bfb5189e05b9eab8dfaad2d1d23f62
category: feature
bugzilla: https://gitee.com/openeuler/riscv-kernel/issues/I99SY1
CVE: NA

--------------------------------

Update T-Head memory type definitions according to C910 doc [1]
For NC and IO, SH property isn't configurable, hardcoded as SH,
so set SH for NOCACHE and IO.

And also set bit[61](Bufferable) for NOCACHE according to the
table 6.1 in the doc [1].

Link: https://github.com/T-head-Semi/openc910 [1]
Signed-off-by: Jisheng Zhang <jszhang@kernel.org>
Reviewed-by: Guo Ren <guoren@kernel.org>
Tested-by: Drew Fustini <dfustini@baylibre.com>
Link: https://lore.kernel.org/r/20230912072510.2510-1-jszhang@kernel.org
Signed-off-by: Palmer Dabbelt <palmer@rivosinc.com>
Signed-off-by: Mingzheng Xing <xingmingzheng@iscas.ac.cn>
```
