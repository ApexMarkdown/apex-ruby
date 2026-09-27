# Guidelines for working with my Fish shell

## When providing instructions for running commands in Fish shell, include:

1. **Shell specification**: Always specify `fish` as the shell
2. **Variable syntax**: Use Fish syntax (`set -gx VAR value`, not `export VAR=value`)
3. **Command substitution**: Use `(command)` not `$(command)`
4. **Conditionals**: Use `if test ... end` not `if [ ... ]; then ... fi`
5. **Path handling**: Use `(pwd)` for current directory, `~` for home
6. **Escaping**: Always escape $ with \$ or use single quotes in regexes

## Execute sandbox commands in Fish shell (not bash):

1. Use Fish syntax: `set -gx VAR value` for environment variables
2. Use `(command)` for command substitution
3. Use `if test ... end` for conditionals
4. Use `source` to execute scripts in current shell context
5. Use `mktemp` to create temporary files safely

## When using heredoc syntax, use one of the following methods (not Bash heredoc syntax):

### Option 1: Direct Execution with Heredoc (Recommended for Simple Cases)

```fish
function setup-env
    fish <<'FISH_SCRIPT'
        set -gx PATH /usr/local/bin $PATH
        set -gx EDITOR vim
        # ... more setup
    FISH_SCRIPT
end
```

**Pros:**
- Simple and direct
- No temporary files
- Good for one-off or simple setups

**Cons:**
- Can be harder to debug
- Limited error handling
- Not ideal for complex multi-step operations

### Option 2: Write to File and Execute (Recommended for Complex Cases)

```fish
function setup-env
    set temp_script (mktemp)

    cat > $temp_script <<'FISH_SCRIPT'
        set -gx PATH /usr/local/bin $PATH
        set -gx EDITOR vim

        # Complex setup logic
        if test -d ~/.local/bin
            set -gx PATH ~/.local/bin $PATH
        end

        # Source additional configs
        if test -f ~/.config/fish/extra.fish
            source ~/.config/fish/extra.fish
        end
    FISH_SCRIPT

    # Execute the script
    fish $temp_script

    # Cleanup
    rm -f $temp_script
end
```

**Pros:**
- Better for complex scripts
- Easier to debug (can inspect the file)
- Can reuse the script file
- Better error handling

**Cons:**
- Creates temporary files
- Slightly more verbose

### Option 3: Use `source` with Process Substitution (Fish 3.0+)

```fish
function setup-env
    source (cat <<'FISH_SCRIPT'
        set -gx PATH /usr/local/bin $PATH
        set -gx EDITOR vim
    FISH_SCRIPT
    | psub)
end
```

**Pros:**
- No temporary files visible
- Direct execution in current shell
- Clean syntax

**Cons:**
- Requires Fish 3.0+
- Process substitution creates a file descriptor (cleaned up automatically)