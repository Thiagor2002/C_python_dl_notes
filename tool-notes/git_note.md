# Git Note

## Table of Contents
- [Git Note](#git-note)
  - [Table of Contents](#table-of-contents)
  - [Git Basics](#git-basics)
  - [Official method](#official-method)
  - [Github初学者操作](#github初学者操作)
  - [Clash 本地代码配置 Git](#clash-本地代码配置-git)
  - [Github操作：多个本地和远程分支管理](#github操作多个本地和远程分支管理)
    - [1. Github的代码管理结构](#1-github的代码管理结构)
    - [2. 从本地获取和上传到不同远程分支操作](#2-从本地获取和上传到不同远程分支操作)
    - [3. 其他](#3-其他)

## Git Basics

git 是以链表的形式存储不同分支的内容变更的，`origin` 表示是在远程分支，不带它是本地分支，通过

1. Make sure your local copy of the selected branch is updated.

   1. Without overwriting anything
      1. fetch down all the branches from that Git remote
         - `git fetch [alias]`
      2. merge a remote branch into your current branch to bring it up to date
         - `git merge [alias]/[branch]`
   2. If you already fetched or you are ready to **overwrite your local copy**, (fetch and merge any commits from the tracking remote branch), then pull (拉取其他分支的内容到本地分支)
      - **`git pull [alias] [branch]`**
      - **`git pull origin [branch]`**
2. Check your repo branches

   1. Local branches
      - `git branch`
   2. All branches on remote repo
      - `git branch -r`
   3. Both local and remote branches
      - `git branch -a`
   4. You can also add `-v` to make the commands explicitly verbose
3. Create a branch and access it

   1. Normal way
      1. create a new branch at the current commit
         - `git branch [new_branch]`
      2. switch to another branch and check it out into your working directory(2 ways)
         - `git checkout [new_branch]`
         - `git switch new_branch`  > Recommended option (avoid `checkout` unless necessary)
   2. Shortcut (2 ways)
      - `git checkout -b [new_branch]`
      - `git switch -c [new_branch]` > Recommended option (avoid `checkout` unless necessary)
4. Get some work done lol
5. Check the status of your work

   - `git status`
6. Did you mess up editing a file and want to restore it to how it was beforehand?

   - `git restore changed_file.txt`
7. Add changes to staging in order to prepare your commit
   (`add` 表示将本地分支的变更内容暂时存起来，而之后的`commit` 则表示提交暂存的变更)
   1. Add a single file
      - `git add [new_file]`
   2. Add all changed files
      - **`git add . -p`**
8. Did you screw up? Reset the staging
   - `git reset` 用于撤销前一次的`add` 暂存操作, 在本地代码仍然存在，只是不再准备提交
   - `git reset HEAD`, 如果只是要撤销单个文件可以使用 `git reset HEAD [file]`
   - `git reset --hard` 表示删除所有未提交(commit) 的修改
9.  Commit
   
   - **`git commit -m "[This is a commit message]"`**

10. Check the commit history of the current branch you're in

    - **`git log`**
    - If you wanna see some cool things with log, you can use something like this:
      - `git log --graph --oneline --all`
11. Make sure you upload your commits to the remote repo! If your local branch is brand new, you must add it to the remote repo.

    1. New branch (Transmit local branch commits to the remote repository branch)
       - **`git push -u origin new_branch`**
    2. Previously existing branch
       - `git push [alias] [branch]`
12. Move to another branch

    - `git checkout [another_branch]`
13. Merge some branch into your current branch (assuming default behavior of pull is merge)

    - `git pull branch_that_will_be_merged_into_current_branch`

For more info check the [GitHub Git Cheat Sheet](https://education.github.com/git-cheat-sheet-education.pdf)


注意: 对于 `add / commit / push` 操作如何撤回的问题：
1. 撤销 add: **`git reset HEAD`** 如前所示（本地分支修改的内容仍然存在，撤销了暂存的内容）
2. 撤销未提交的代码修改（修改了文件但是没有 `commit`）:
   1. `git checkout -- [file]` 表示撤销单个文件
   2. `git checkout --` 表示撤销全部修改，使得工作区恢复到最近一次 commit，本地修改的内容会删除
3. 还未 `push` 的情况下 撤销最近一次 `commit`: 
   1. **`git reset --soft HEAD~1`** 会撤销 commit 但是保留 add 暂存的修改, 代码仍然在 staging
   2. **`git reset --mixed HEAD~1`** 表示 撤销 commit+add, 保留本地代码修改，回到工作区
   3. `git reset --hard HEAD~1` 表示 彻底删除 commit+add+本地代码， 例如`A -> B -> C (HEAD)` 执行该指令会变成 `A -> B`
4. 撤销已经 push 的 commit
   1. **`git revert HEAD`** 这会创建一个 新的 commit 抵消旧 commit，不会修改历史，适合共享仓库
   2. `git reset --hard HEAD~1 && git push -f` 这会回退远程仓库(修改远程历史可能会影响其他分支协作者，谨慎！)
5. 查看 commit 历史（撤销前建议）: `git log --oneline`
   例如执行该指令后输出结果是
   ```
   a3f4c2d fix bug
   93f0abc add feature
   ```
   则可以可以执行该回退指令: `git reset --hard 93f0abc`


## Official method

_[Reference1:Git-on-the-Server-Getting-Git-on-a-Server](https://git-scm.com/book/en/v2/Git-on-the-Server-Getting-Git-on-a-Server)_
_[Reference2:create-a-remote-git-repo-from-local-folder](https://stackoverflow.com/questions/14087667/create-a-remote-git-repo-from-local-folder)_

## Github初学者操作

**Reference Source:** [GitHub初学——上传、修改本地项目文件到GitHub](https://blog.csdn.net/Demonslzh/article/details/104268334)

1. 首先打开一个GitHub的Repository（或者新建一个，新建repository的按钮打开主页可以直接看到）
2. 回到本地，打开我们的项目所在文件
   - 点击右键，选择 Git Bash Here(linux 和 windows 虚拟机自带git指令， windows 系统可以下载 git 软件)
   - 输入 `git init`，初始化本地仓库
   - `git clone https://github.com/username/repository.git`，将远程仓库克隆到本地
     (还有一个指令更常用  `get clone git@github.com: username/repositor.git`， 可以通过github 项目主页的 code -> https/ ssh， 获得下载链接)
   - `cd repository` 进入仓库
3. 修改并上传项目
   - 如果是第二次或第三次上传，`git pull origin main`(该指令表示拉取远程 main 分支的内容)
   - `git add .`，添加所有文件
   - `git commit -m "message"`， 提交修改注释
   - `git push -u origin main`， 上传到远程仓库(带 origin 表示远程，没有 origin 是本地分支)
4. 其他操作
   - `ssh -T git@github.com`，测试连接， 如果成功连接到github会看到如下的输出: `<p>Hi YongDeng715! You've successfully authenticated, but GitHub does not provide shell access.``</p>`

Github 如何连接上电脑或服务器：
window电脑可以下载 git portable 软件，linux 系统有 git 操作指令

使用 SSH 代替 HTTPS 进行验证的步骤(使用SSH下载或上传文件需要本地电脑得到github验证, 需要将本机的ssh/gpg密钥添加到github中)：

1. 生成 SSH 密钥:

```bash
   ssh-keygen -t rsa -b 4096 -C "your_email@example.com"
```

2. 添加 SSH 密钥到 SSH 代理：

```bash
   eval "$(ssh-agent -s)"
   ssh-add ~/.ssh/id_rsa
```

3. 复制公钥内容，将公钥添加到 Github:
   ``cat ~/.ssh/id_rsa.pub``
4. `ssh -T git@github.com`，测试连接， 如果成功连接到github会看到如下的输出: `<p>Hi YongDeng715! You've successfully authenticated, but GitHub does not provide shell access.``</p>`

GPG 密钥用于签署和验证 Git 提交的真实性。以下是生成 GPG 密钥并配置 Git 以使用 GPG 密钥的方法：

1. 确保系统上安装了GPG， 如果没有安装，可以采用以下命令：
   ```bash
   sudo apt-get install gnupg # in Ubuntu/Debian platform
   brew install gnupg # in macOS platform
   ```
2. 生成 GPG 密钥 ``gpg --full-generate-key``
   根据提示选择密钥类型、大小和有效期，并提供你的用户信息（如姓名和电子邮件）。例如：
   - 类型：RSA and RSA (default)
   - 密钥大小：4096
   - 有效期：0（表示永不过期）
   - 用户信息：输入你的姓名和电子邮件`
3. 列出 GPG 密钥： ``gpg --list-secret-keys --keyid-format LONG``
   可以看到类似的输出：
   ```bash
   /home/username/.gnupg/secring.gpg
   -------------------------------
   sec   4096R/ABCDEF1234567890 2024-06-18 [expires: 2025-06-18]
   uid                          Your Name <you@example.com>
   ssb   4096R/1234567890ABCDEF 2024-06-18
   ```

4. 导出你的公钥，并将其添加到 GitHub： ``gpg --armor --export ABCDEF1234567890``

## Clash 本地代码配置 Git
reference: [配置Git 使用Clash 代理](https://lihuaxianxue.space/post/8c98b106)

1. 关于常规检查
检查 git 安装版本 以及 clash 代理端口

终端 -> `git --version` -> `git version 2.40.0.windows.1` 表面已经安装
clash 主页面 -> 常规 -> 端口 可以 查看 `mixed-port` 端口号
或者检查配置文件，通常为 7890 端口
```plantext
mixed-port: 7890
```

2. 配置 Git 使用 Clash 代理
* 打开命令行（CMD 或 Git Bash）。
* 设置 Git 的全局代理，使用 HTTP 协议（因为 Clash 的 mixed-port 支持 HTTP）：
```bash
git config --global http.proxy http://127.0.0.1:7890
git config --global https.proxy http://127.0.0.1:7890
```
* 关于上述指令解释：
  * `http.proxy` 和 `https.proxy`：设置 HTTP/HTTPS 协议的代理（GitHub 使用 HTTPS）;
  * `127.0.0.1:7890`：Clash 的本地代理地址和端口

* 检查 Git 是否正确应用了代理设置：
```bash
git config --global --get http.proxy
git config --global --get https.proxy
# 返回 `http://127.0.0.1:7890`，说明配置成功
```
4. 测试代理连接

测试 Git 是否能通过代理访问 GitHub：
```bash
git ls-remote https://github.com/{User}/{RepositoryName}.git
```

成功输出示例:
```PLANTEXT
00c87001b3c99bec7a6f1ef4b342c9a4f3d668d7        HEAD
00c87001b3c99bec7a6f1ef4b342c9a4f3d668d7        refs/heads/main
...
```

如果失败（例如超时或连接错误），检查 Clash 是否运行，或者端口是否正确

5. 常见问题： Clash 未运行，或端口未监听 从而无法连接到 127.0.0.1:7890

如果端口被占用，修改 Clash 配置中的 mixed-port（例如改为 7891），然后更新 Git 配置:
```bash
git config --global http.proxy http://127.0.0.1:7891
git config --global https.proxy http://127.0.0.1:7891
```

不需要代理时如何取消（清除代理设置指令）：
```bash
git config --global --unset http.proxy
git config --global --unset https.proxy
```

6. 使用环境变量临时设置

CMD 指令：
```bash
set HTTP_PROXY=http://127.0.0.1:7890
set HTTPS_PROXY=http://127.0.0.1:7890
git clone https://github.com/{User}/{RepositoryName}.git
```

Git Bash 指令：
```bash
export HTTP_PROXY=http://127.0.0.1:7890
export HTTPS_PROXY=http://127.0.0.1:7890
git clone https://github.com/{User}/{RepositoryName}.git
```

## Github操作：多个本地和远程分支管理

### 1. Github的代码管理结构

GitHub的代码管理基于分布式版本控制，核心是分支模型。主流分支策略包括Git Flow、GitHub Flow和Trunk-Based Development。工业级团队通常采用Git Flow或其变体，以支持并行开发、版本发布和紧急修复。

- **主分支 (main/master)**: 生产就绪代码，代表最新稳定版本。
- **开发分支 (develop)**: 集成分支，用于合并特性分支，准备发布。
- **特性分支 (feature/xxx)**: 短期分支，用于开发新功能，从develop分支创建。
- **发布分支 (release/xxx)**: 预发布分支，用于最终测试和bug修复，从develop创建。
- **热修复分支 (hotfix/xxx)**: 紧急分支，从main创建，用于生产bug修复。

### 2. 从本地获取和上传到不同远程分支操作

#### 特性分支创建与同步规则
1. **创建特性分支**:
   - 从develop分支创建: `git checkout -b feature/new-feature develop`
   - 推送至远程: `git push -u origin feature/new-feature`
2. **同步最新代码**:
   - 定期合并develop到特性分支: `git merge develop` 或 `git rebase develop`
   - 避免长时间分支以减少冲突。
3. **提交特性分支**:
   - 本地提交: `git add . && git commit -m "feat: add new feature"`
   - 推送: `git push origin feature/new-feature`
4. **合并回develop**:
   - 创建PR/MR，代码评审通过后合并。
   - 合并后删除分支: `git branch -d feature/new-feature && git push origin --delete feature/new-feature`

#### 代码冲突预处理方案
- **预防冲突**: 每日同步主干，保持分支短寿（<5天）。
- **检测冲突**: 在推送前运行 `git fetch && git rebase origin/develop`。
- **解决冲突**:
  1. 编辑冲突文件，保留正确版本。
  2. `git add <file>` 标记解决。
  3. `git rebase --continue` 或 `git commit` 完成。
- **最佳实践**: 使用 `git mergetool` 工具辅助解决。

#### PR/MR提交流程
1. **创建PR**: 在GitHub/GitLab上，从特性分支到develop分支创建PR。
2. **描述**: 填写标题、描述变更内容、关联issue。
3. **评审**: 至少1-2名评审者，检查代码质量、测试覆盖。
4. **CI检查**: 确保所有自动化测试通过。
5. **合并**: 使用Squash Merge保持历史干净，合并后删除分支。

#### 代码评审卡点约定
- **评审标准**: 代码规范、性能、安全性、测试完整性。
- **卡点**: 未通过CI、代码覆盖率<80%、安全漏洞。
- **反馈**: 评审者提供具体建议，开发者响应并修改。
- **审批**: 评审通过后方可合并。

#### 主干同步机制
- **每日同步**: 所有分支从main/develop拉取最新代码。
- **自动化**: 配置CI在PR时检查与main的兼容性。
- **回流**: 热修复分支修复后，同时合并到develop和main。

#### 紧急Hotfix分支处理逻辑
1. **创建**: 从main创建 `git checkout -b hotfix/critical-fix main`
2. **修复**: 快速修复bug，提交并测试。
3. **合并**: 直接合并到main和develop: `git checkout main && git merge hotfix/critical-fix`
4. **标签**: 发布新版本 `git tag v1.1.1`
5. **清理**: 删除分支 `git branch -d hotfix/critical-fix`

#### 版本发布分支管理
1. **创建发布分支**: 从develop创建 `git checkout -b release/v1.0 develop`
2. **稳定化**: 仅允许bug修复，禁止新功能。
3. **测试**: 运行完整测试套件。
4. **发布**: 合并到main `git checkout main && git merge release/v1.0`，打标签 `git tag v1.0`
5. **回流**: 合并回develop `git checkout develop && git merge release/v1.0`
6. **清理**: 删除分支。

#### 常见团队协作踩坑规避方案
- **分支过长**: 设置分支生命周期上限，强制评审。
- **强制推送**: 禁用force push到共享分支。
- **未同步**: 要求每日rebase。
- **历史混乱**: 使用Conventional Commits规范提交信息。
- **权限管理**: 配置分支保护，只允许PR合并。

### 3. 其他

- **工具集成**: 使用GitHub Actions或GitLab CI/CD自动化流程。
- **培训**: 团队成员熟悉分支策略，避免误操作。
- **文档**: 维护CONTRIBUTING.md记录规则。
