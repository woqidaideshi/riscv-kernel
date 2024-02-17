[English](./README.en.md) | 简体中文

#### 介绍

目前各个 RISC-V SoC 厂商维护的 kernel 版本并不一致，而 openEuler 系统要求每个版本统一内核。这导致基于各种开发板发布的各种操作系统版本都是内核不一致的第三方版本，增大了维护的难度并且带来了生态的分裂。riscv-kernel 目标是针对 RISC-V 架构在 openEuler 建立统一的 kernel 生态，共享欧拉生态建设与影响。该项目处于开发中，欢迎各方力量积极贡献。

#### 软件架构

riscv64

#### 项目意义

- 同源内核可提升用户在不同硬件平台的用户体验。
- 加强硬件厂商、发行版方和开发者之间的合作，提高内核的兼容性和开发效率。
- 降低各方的开发和维护成本，加快应用和生态系统的发展。
- 促进 RISC-V 硬件平台的发展和推广，为 RISC-V 生态系统的全面发展奠定基础。

#### 说明

- 本仓库基于 [openeuler/kernel](https://gitee.com/openeuler/kernel) 的 OLK-6.6 分支建立，并持续同步更新。
- 在未合入 OLK-6.6 之前，将以 patch 形式合入 [src-openeuler/kernel](https://gitee.com/src-openeuler/kernel) 并维护。
- 欢迎提交不同 RISC-V SoC 的支持补丁，如果遇到代码冲突等问题，可提交 issues 并持续跟踪。

#### 参与贡献

1.  Fork 本仓库
2.  基于 OLK-6.6 新建开发分支
3.  制定新增支持能力范围和计划
4.  提交 SoC 支持
5.  解决 PR 所产生的代码冲突

