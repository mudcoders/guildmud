# How to contribute

First off, thanks for taking the time to contribute! The following is a set of guidelines for contributing to GuildMUD. These are just guidelines, not rules, so use your best judgement and feel free to propose changes to this document in a pull request.

## Code of Conduct

This project adheres to the [Contributor Covenant 1.2](http://contributor-covenant.org/version/1/2/0). By participating, you are expected to uphold this code. Please report unacceptable behavior to [zach@mudcoders.com](mailto:zach@mudcoders.com).

## Getting Started

This codebase is built on top of the barebones [SocketMUD](http://www.mudpedia.org/mediawiki/index.php/SocketMUD) codebase. If you are new to the C programming language, or MUDs in general, head on over to [MudBytes](http://www.mudbytes.net/), [/r/MUD](https://www.reddit.com/r/MUD), and [The MUD Coders Guild](http://signup.mudcommunity.com/) to get acquainted (and meet some uncomfortably enthusiastic people in the process).

- Make sure you have a [GitHub account](https://github.com/signup/free).
- For issues or feature requests, submit an issue on GitHub (assuming one does not already exist).
  - Clearly describe the issue including steps to reproduce when it is a bug.
  - Make sure you fill in the earliest version that you know has the issue.
- Fork the repository on GitHub.

## Making Changes

- Create a feature branch from where you want to base your work.
  - This is usually the `develop` branch.
  - Only target release branches if you are certain your fix must be on that branch.
  - Please avoid working directly on `develop` branch.
- Make commits of logical units.
- Make sure your commit messages are clear and concise.
- Make sure you have added the necessary tests for your changes.
- Run _all_ the tests to assure nothing else was accidentally broken.

## Submitting Changes

- Push your changes to a feature branch in your fork of the repository.
- Submit a pull request to the repository. Be sure to include references to any GitHub issues related to the change.
- After feedback has been given we expect responses within two weeks. After two weeks the pull request may be closed if it isn't showing any activity.
