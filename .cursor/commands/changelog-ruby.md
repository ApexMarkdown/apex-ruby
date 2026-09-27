Review all staged and unstaged files in the repo, as well as untracked files (source code files only, excluding untracked Markdown, image, and temp files). Write a commmit message that uses @ labels to specify what type of change each line is. Apply @new, @fixed, @changed, @improved, and @breaking as appropriate to each line. Only add @ labels to changes that affect the user, not technical details. Technical details can be included in the commit, just don't add @ labels to those lines. Be sure to include a general description (< 60 characters) as the first line, followed by a line break.

Do not add @tags to notes about documentation updates. Always focus on actual code changes we've made since the last commit when generating the commit message.

Always use straight quotes and ascii punctuation, never curl quotes. Don't use emoji.

Don't hard wrap the @tagged lines. Keep each change on one line, no matter how long it gets.

Always include a blank line after the first line (commit message) before the note.

Save this commit message to commit_message.txt. {% if args.update %}Update the file, merging changes, if file exists, otherwise create new. {% else %}. Overwrite existing contents.{% endif %}