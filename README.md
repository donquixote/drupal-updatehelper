# drupal-updatehelper
Drupal module that suggests drush and git commands for module update, in a UI on `admin/reports/updates/drush`.

In this early version, the generated commands are like "take it or leave it", without much added explanation.

## Assumptions

The module, in its current version, is designed for a scenario where:

- The entire site is in one big git repository.
- Contrib modules live under `sites/all/modules/contrib/`.
- You want to use drush to download the new module versions.
- You want dedicated commits for each module update.
- Sometimes you only want to apply security updates, another time you want to update everything.
- Some contrib modules may have local modifications ("hacks"), that have been committed to the repository, and that you want to preserve on module update.
  You want all hacks to be clearly visible in the git history, e.g. by screaming "HACK xyz" in all-caps in the commit message.
  You are not sure if someone else, or your past self, committed such hacks in the past, without properly saying so in the commit message.

The authors of this module make no judgement or recommendation about whether or not working like this is a good idea.

## Steps

The idea is that you proceed as follows:

- Download an _unmodified_ copy of each module, in the version that is currently installed, e.g. `drush dl xyz-7.x-1.2`.
- Commit the changes as e.g. "UNHACK module xyz".
- Download the new version, using `drush dl xyz-7.x-1.6`.
- Commit the changes as e.g. "(up) xyz 7.x-1.2 -> 7.x-1.6."
- Restore the hacks, if any:
  - Using interactive rebase, `git rebase -i`, to remove the "UNHACK" commit, OR.
  - Using `git revert HEAD^`, to revert the UNHACK commit, possibly with a suitable commit message.
- Manually adjust the commit message as "(up) xyz 7.x-1.2 -> 7.x-1.6, preserving local changes.", if applicable.
- Run `drush updb` to apply database updates.


When working this way, the following can be annoying and time-consuming:

- Type all module names that need update.
- Look up the current version of the module to download the unmodified copy.
- Look up the old and new version, to mention them in the commit message.

## How this module helps

This module does not completely replace all manual commands.

However, it does automate the most annoying steps and prepares the commands for you.

## Limitations

The current version has some limitations:

- Drupal core is treated as if it was a contrib module. You need to manually take care of that.
- Themes are treated like modules.
- All module packages are treated as if they live inside `sites/all/modules/contrib/`.
- The module does not check the output of `git status`, to determine which modules are hacked. This is up to you.
