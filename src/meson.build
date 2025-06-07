sources = files('main.cpp')

git_current_short = run_command(['git', 'rev-parse', '--short', 'HEAD'], check : false)
if git_current_short.returncode() == 0
  git_current_short = git_current_short.stdout().strip()
else
  message('Warning: Git current short could not be determined. Using "unknown".')
  git_current_short = 'unknown'
endif

git_branch_name = run_command(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], check : false)
if git_branch_name.returncode() == 0
  git_branch_name = git_branch_name.stdout().strip()
else
  # Handle cases where git is not available or not a git repository
  message('Warning: Git branch could not be determined. Using "unknown".')
  git_branch_name = 'unknown'
endif

build_time = run_command(['date', '+%Y-%m-%d %H:%M:%S'], check : false)
if build_time.returncode() == 0
  build_time = build_time.stdout().strip()
else
  message('Warning: Build time could not be determined. Using "unknown".')
  build_time = 'unknown'
endif

git_is_dirty = run_command(['git', 'status', '--porcelain'], check : false)
  if git_is_dirty.returncode() == 0
    # Command ran successfully. Check if its standard output is empty.
    if git_is_dirty.stdout().strip() == ''
      # Output is empty, meaning the Git repository is clean.
      git_is_dirty = ''
    else
      # Output is NOT empty, meaning there are uncommitted changes (dirty).
      git_is_dirty = '1'
    endif
  else
    message('Warning: Git is not available or not a git repository. Assuming clean.')
    git_is_dirty = ''
endif

configure_file(
  input : 'version.h.in',
  output : 'version.h',
    configuration : {'GIT_BRANCH_NAME': git_branch_name, 'GIT_CURRENT_SHORT': git_current_short, 'BUILD_TIME': build_time, 'GIT_IS_DIRTY': git_is_dirty}
)
executable('utild', sources, dependencies: [lipc_dep], cpp_args: ['-static-libstdc++'], link_args: ['-static-libstdc++'])
