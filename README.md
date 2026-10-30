# MFLAB Linux C

Linux C source tree for the MFLAB lower-computer application.

This initial codebase is derived from the previous M005M/L500K source tree. MFLAB development will continue from this base, but the repository is not intended to keep maintaining the old M005M/L500K product lines. When future changes touch legacy-series-specific code, it can be removed or simplified as needed to keep the MFLAB codebase clean.

# 丢弃所有已跟踪文件的本地未提交修改（包括暂存区）
git restore --source=HEAD --staged --worktree .

# 拉取当前分支的最新代码
git pull --ff-only

# 获取远程最新分支信息
git fetch origin

# 创建本地分支、切换过去，并跟踪对应远程分支
git switch -c lw/hw_0818_scope_1023 --track origin/lw/hw_0818_scope_1023

# 确认当前分支和工作区状态
git status --short --branch

