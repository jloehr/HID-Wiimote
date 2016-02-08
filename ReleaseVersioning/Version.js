var FSO = new ActiveXObject("Scripting.FileSystemObject");

var ForReading = 1;

function ParseVersionStringPart(VersionString, Index)
{
    var Number = parseInt(VersionString[Index]);
    if(isNaN(Number))
    {
        return 0;
    }

    return Number;
}

function ParseVersionString(VersionString)
{
    var VersionParts = VersionString.split(".");
    var Version = {
        String: VersionString,
        Major: ParseVersionStringPart(VersionParts, 0),
        Minor: ParseVersionStringPart(VersionParts, 1),
        Build: ParseVersionStringPart(VersionParts, 2),
        Revision: ParseVersionStringPart(VersionParts, 3)
    }

    return Version;
}

function GetVersionsFromReadme(ReadmePath)
{
    // First Line of Readme is either
    // HID Wiimote u.v.w.x
    // or
    // HID Wiimote u.v.w.x (Driver Package a.b.c.d)

    var ReadmeFile = FSO.OpenTextFile(ReadmePath, ForReading);
    if (ReadmeFile == null)
    {
        WScript.Echo("Can't find Readme File: " + ReadmePath);
        WScript.Exit(1);
    }

    var FirstLine = ReadmeFile.ReadLine();
    if (FirstLine.length == 0)
    {
        WScript.Echo("First line of Readme is empty: " + ReadmePath);
        WScript.Exit(1);
    }

    var FirstLineParts = FirstLine.split(" ");
    var VersionString = FirstLineParts[2];
    if (VersionString.length == 0) {
        WScript.Echo("Version string of Readme is empty: " + ReadmePath);
        WScript.Exit(1);
    }

    var Versions = {
        ControlCenter: ParseVersionString(VersionString)
    }
    
    if (FirstLineParts.length > 3)
    {
        VersionString = FirstLineParts[FirstLineParts.length - 1].slice(0, -1);
        Versions.DriverPackage = ParseVersionString(VersionString);
    }
    else
    {
        Versions.DriverPackage = Versions.ControlCenter;
    }
    

    return Versions;
}

function GenerateDriverPackageVersionPropsSheet(DriverPackageVersion, OutputPath)
{
    var FileContent = "";
    FileContent += '<?xml version="1.0" encoding="utf-8"?>' + "\n";
    FileContent += '<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">' + "\n";
    FileContent += '  <PropertyGroup Label="UserMacros">' + "\n";
    FileContent += '    <DriverPackageVersion>' + DriverPackageVersion.String + '</DriverPackageVersion>' + "\n";
    FileContent += '  </PropertyGroup>' + "\n";
    FileContent += '</Project>' + "\n";

    var OutputFile = FSO.OpenTextFile(OutputPath);

    if ((OutputFile == null) || (OutputFile.ReadAll() != FileContent))
    {
        OutputFile = FSO.CreateTextFile(OutputPath, true);
        OutputFile.Write(FileContent);
    }
}