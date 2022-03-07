Falcon mod by Caffeinic/Yakizakana, DARTH--VADER, and Crafty
-----------------------------------------------------------------------------

This is the client component of the Falcon mod. It does not include any of the changes required to implement the server functionality.


Setup
-----------------------------------------------------------------------------

NOTE: The launcher installs Falcon automatically. These steps are for a manual, no-launcher install.

1. Copy the included Mods folder to your main Crysis Wars install directory. The default is C:\Program Files (x86)\Electronic Arts\Crytek\Crysis Wars
2. Modify your game shortcut properties to include -mod Falcon.
3. Start the game and join a server. You should be able to perform the wall jump in the same way as Crysis 1.


Features
-----------------------------------------------------------------------------

1. Wall jump

    The wall jump is a movement trick that allows you to jump 2-3x as high as usual when facing a wall with fists out. It works by exploiting the "push off" effect to accelerate upwards.
    This bug originally exsited in Crysis 1, but was fixed in Crysis Wars. This modification reverts the changes made in Crysis Wars so that wall jump works again.
    fn_wallJumpMultiplier [0 = Disabled (default), 1 = Crysis 1 walljump]

    This CVAR is a multiplier, so any nonzero value is valid. Sane values are 0-5. This CVAR is server controlled.

2. Console chat commands

    chat <message>
        Says <message> in chat. This is intended as a convienience option to make chat command binds possible without the use of keyboard macros.

    chatt <message>
        Says <message> in team chat. This is identical to chat but the message is only visible to your team.

3. Falcon RCON

    sc <command>
        Executes <command> in the server console. NOTE: this only works if you have been granted admin on the server.

    scl <number of lines>
        Gets and prints <number of lines> of the server log to the client console. NOTE: this only works if you have been granted admin on the server.

4. Constant mouse sensitivity

    This CVAR prevents weapon mass from affecting mouse sensitivity. This is set clientside in autoexec.cfg.
    fn_ConstantMouseSensitivity [0 = Disabled (default), 1 = Enabled]

5. Weapon mass adjustment

    This feature multiplies the mass of all weapons by a constant. This CVAR is server controlled.
    fn_weaponMassMultipler [1 = No change (default)]

6. Mine/Claymore planting fix

    This feature fixes the bug where mines and claymores fail to be planted.
    It might still fail if your FPS is above 400-500, but this should fix it for most people.
    fn_fixExplosivePlant; [1 = Enabled (default), 0 = Disabled]

7. Custom characters

    This feature allows you to change your character's appearance to look like many of the characters from Crysis single player.
    It only works on supported servers which also use Falcon.
    The chat command !characters will list the options, and !setcharacter <name> can set your current character

8. Client controlled 1st person body

    To make custom characters work, it was required to globally disable 1st person body rendering.
    If you want to override this for default characters, you can use this option
    fn_enableFpBody [0 = Disabled for all characters (default), 1 = Enabled for default characters]

9. Engine FPS cap

    Use this option to set an FPS cap from within the engine.
    This method does not introduce input lag like 3rd party FPS limiters (such as RTSS, or the options in GPU driver control panels)
    sys_MaxFps [0 = No limit (default), nonzero = FPS capped at this value]
    Requires Crysis.exe from jedi95's CW-Launcher 2.5 or higher to have any effect.

10. Clientside FOV control

    This option allows you to change the FOV on the client
    fn_fov [new FOV, default 60]
    The maximum FOV is 75, the minimum is 50, and the default is 60

11. Shoot zoom effect disable

    Allows you to disable the gradual "zoom in" effect while firing automatic weapons
    fn_disableShootZoom [0 = No change (default), 1 = Enabled (no zoom effect occurs)]


Debugging
-----------------------------------------------------------------------------

If you can't seem to do wall jumps even after installing the mod the first step is to verify that the mod is being loaded.

You should see "Falcon" text in the bottom right corner of the menu next to the game version.

Another way to check the version is to open Game.log in your Crysis Wars directory and look for a line like this:
[Falcon] Loaded Falcon Client version 0.800 SSE2 0000000

If you see this line, then Falcon loaded correctly.


Compiling
-----------------------------------------------------------------------------

This project is built with VS2005. Using a newer version of Visual Studio may result in stability issues.
