# GuildMUD

GuildMUD is a SocketMUD-based Multi-User Dungeon built and managed by the members of [The MUD Coders Guild](https://mudcoders.com/).

## Development

The GuildMUD development environment runs on a VirtualBox virtual machine that is managed using Vagrant. Additionally, the following packages and tools are required to properly run this application:

- [Vagrant](https://www.vagrantup.com/) - Development Environment Orchestration
- [VirtualBox](https://www.virtualbox.org/) - Virtualization Platform

### Development Environment

While it is recommended to familiarize yourself with basic [Vagrant command syntax](https://www.vagrantup.com/docs/cli/), with the two dependencies above installed, spinning up the GuildMUD development environment is as simple as running one command:

`vagrant up`

This command will perform a host of automated tasks to get a development environment up and running:

- Power up the virtual machine
- Add `guildmud.dev` to the host machine's hosts file
- Install any necessary dependencies within the virtual machine (Make, GCC, GDB, Valgrind, etc)

Once the Vagrant box has booted, the virtual machine can be logged into using the `vagrant ssh` command, and the game server (when running) can be accessed at `telnet://guildmud.dev:9009`.

When you are finished coding, the `vagrant halt` command will shut the virtual machine completely down, while `vagrant suspend` will merely pause it for later. Running `vagrant up` will bring it back up again.

**Note: If for whatever reason the development environment isn't working as expected, you can re-provision it with the `vagrant provision` command, or you can burn it down and start over using `vagrant destroy` followed by `vagrant up`.

#### MacOS development environment

For macOS there's no need to setup vagrant, and having macOS Xcode installed including the Command Line tools plus some brew packages is enough.

The easiest way to install XCode with the Comman Line Tools is, at the command line term:

`$ xcode-select --install`

once this is done, proceed to install [brew](https://brew.sh/) (if not installed)

and install the following packages:

`$ brew install sqlite check` 


### Compiling GuildMUD

To compile and start GuildMUD, first log into the development machine using the `vagrant ssh` command and then navigate to the `./src` directory. Then, simply run the `make` command to compile the `guildmud` binary.

### Launching GuildMUD

After compiling GuildMUD, the server can be launched from within the `src` directory. This can be accomplished by typing `./guildmud` in the terminal. This is a long running process, so to shut down the server simply press `Ctrl+C` or close the terminal window.

With the GuildMUD server running, you can connect to it from any standard MUD client from the host server at `guildmud.dev:9009`.

## Contributing

If you would like to contribute to Maelstrom, please review the [CONTRIBUTOR](.github/CONTRIBUTING.md) guidelines.

## License

GuildMUD is open-sourced software licensed under the [MIT license](http://opensource.org/licenses/MIT).
