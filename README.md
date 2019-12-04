IRRPT
======

### Sponsored by TorIX - The Toronto Internet Exchange
* Originally written by Richard A Steenbergen <ras@e-gerbil.net>
* Older versions can be found on sourceforge at [http://sourceforge.net/projects/irrpt/](http://sourceforge.net/projects/irrpt/)
* IPv6 support added by Elisa Jasinska <elisa@bigwaveit.org> for version 2.0 
* Bug Fixes provided by Anna Claiborne <domino@theshell.org> for version 2.0


Summary
-------

A collection of tools which allow ISPs to easily track, manage, and 
utilize IPv4 and IPv6 BGP routing information stored in Internet Routing 
Registry (IRR) databases. Some of these tools include automated IRR data 
retrieval, update tracking via CVS, e-mail notifications, e-mail based 
notification for ISPs who still do human processing of routing information, 
and hooks for automatically deploying prefix-lists on routers.


Purpose
-------

Internet Routing Registry (IRR) services have existed in various forms for 
some time, yet many ISPs (particularly outside of the European/RIPE 
region) have still not adopted it. There are a variety of reasons given, 
but some of the most important ones include:

1. The system is overly complicated, and lacks sufficient examples. End 
   users can not figure it out, which means another layer of support 
   structure must be added, or proxy registration must be implemented.
2. A publicly accessable description of every import and export policy 
   to every transit, peer, and customer, is difficult to maintain, and 
   is not in the best business interests of many ISPs.
3. There are no existing tools which provides registration change 
   tracking. Without this kind of tracking, there is not enough 
   accountability for prefix registrations, and router configuration 
   updates are difficult to manage.

This tool takes a pragmatic approach, by focusing on the key elements and 
critical goal of the IRR, in order to provide simple and effective prefix- 
list management for the masses. To that end, IRR PowerTools aims to 
provide the following functionality:

   * Automated retrieval of prefixes registered behind an IRR Object.
   * Automatic exclusion of bogon or other configured undesirable routes.
   * Tracking and long-term recording of prefix changes through CVS.
   * Automatic aggregation to optimize data and reduce unnecessary changes.
   * E-mail updates, letting users know that their change was processed.
   * E-mail alerts to the ISP, letting them know of new routing changes.
   * Exporting of change data in e-mail form, for non-IRR using ISPs.
   * Router config generation, for easy automated config deployment.

While many ISPs still rely on error-prone e-mail and human processing to 
handle prefix-lists, a few have developed systems similar to this one in 
order to automate their prefix-list management. Unfortunately each of 
these systems is proprietary and owned by the respective ISPs, leaving 
other networks with a choice between spending money and man-hours writing 
duplicating this work, or more often taking the path of least resistance 
and simply using e-mails and human processing.

This project aims to provide to every network the same features that 
individual ISPs have been developing internally for years. By making it 
easier for every ISP to effectively utilize the IRR data, we hope to 
increase the popularity of the IRR as a prefix-list management tool, which 
will hopefully lead to more accurate data and a more effective system.


Requirements
------------

* PHP          >= 5.6     (http://www.php.net/downloads.php)
* CVS                     (http://www.gnu.org/software/cvs/)
* CVSweb       Optional   (http://people.freebsd.org/~scop/cvsweb/)


Installation
-------------

Make sure you have the required packages as listed above installed. Clone 
the git respository in your desired install path via:
https://github.com/6connect/irrpt.git
Or, download the zip file and extract.
Run 'php configure.php' in the base installation directory.

To get started, you probably want to take a look at every file in 
the /conf directory. The most important information to change will be 
paths and company-specific information such as the name, ASN, and 
e-mail addresses.

    irrpt.conf      - This is the master config file which contains global 
                      configuration information, the paths to our internal 
                      locations and external tools, and internal parameters.

    irrdb.conf      - This is the second most important config file, 
                      containing a list of the ASNs and IRR Objects you wish
                      to track.

    nag.conf        - This file contains the settings for the nag process.

    nag.msg         - This file contains the message that will be sent 
                      during the nag process.

    exclusions.conf - This file lists routes which can not be registered. A 
                      good example for this file would be known bogon routes,
                      or routes in known unallocated space.

The irrdb.conf file should certain a unique ASN, the IRR object (an 
AS-SET or AUT-NUM record) that you are interested in tracking, and a 
contact e-mail for change notification. You probably want to track your 
own ASN and AS-SET record here as well.

A reasonable deployment would be to crontab the fetch process once or 
better yet twice a day. If you need to add a new customer outside of 
the normal fetch schedule, or if a customer needs an emergency 
prefix-list update, you can add the ASN/IRR Object to irrdb.conf and 
run a manual pull of just that ASN, with "./fetch ASN".

After the updates are processed, you should receive a local copy of the 
e-mail. It is probably reasonable to keep a human being in the loop 
between prefix fetching and prefix deployment, to make certain that 
nothing "bad" or unintended is happening. After you are reasonably 
certain that the changes are ready for deployment, you can generate 
the router configs use the "pfxgen" tool. Optionally, you can nag 
any of your transit providers who still require e-mail updates using 
the nag process.

If there is anything else that you can't figure out, it is probably 
either a bug or an oversight in the documentation. Send e-mail about 
either one, and I'll make certain it gets addressed in a future 
release.


Documentation
-------------

The operation of IRR PowerTools is broken down into the following distinct 
operations:

### irrpt_fetch

    $ bin/irrpt_fetch -h
    Usage: bin/irrpt_fetch [-h46qv] [-f file] [--nocvs] [object]

Quiet mode:

    $ bin/irrpt_fetch -q 42
    ...

Verbose mode:

    $ bin/irrpt_fetch -v 42
    ...

irrpt_fetch uses the list of objects provided by irrdb.conf by
default.  To track additional objects, simply add them to the 
irrdb.conf file.  To fetch objects for a specific AS, or specific
object, provide this as an argument per the above example.

This is the stage where you pull data for the objects that you are 
tracking off of the IRR, and store them locally. Inside the fetch
process, the following steps are performed:

1. Query a IRR whois server for prefixes behind the IRR object.
2. Match prefixes (including more-specifics) against the Exclusions 
   list, which contains prefixes that can not be registered. 
3. Store the approved prefixes locally.
4. Run the prefixes through an aggregation tool to optimize them.
5. Track changes to both the raw and aggregated prefix lists via CVS.
6. When changes are detected, send out notification e-mails to 
   customers and optionally a local copy to your operations staff, 
   alerting everyone that the routing change has been successfully 
   processed.

### irrpt_nag

    $ bin/irrpt_nag -h
    Usage: bin/irrpt_nag [-hp] [-c config] [-m message] <previous rev> <current rev>

    Options:
       -p    Preview mode (for diagnostic use). Print results to screen instead of
             e-mail.

In this stage, any transit providers (or other interested parties) 
who still track prefix-list updates via e-mail rather than via IRR 
can be notified of the change.

As more providers begin to use IRR, and rely on it as their primary 
prefix tracking tool, this should become less and less necessary. 
Unfortunately many of the largest ISPs still use human processing of 
prefix lists, so for the immediate future you will probably still be 
getting a lot of use out of this tool.

### irrpt_pfxgen

    $ bin/irrpt_pfxgen -h
    Usage: bin/irrpt_pfxgen [-h46] [-p pfxstr] [-p6 pfxstr_v6] [-l pfxlength] [-l6 pfxlength_v6] [-f format] <asn>
      pfxstr       - The prefix-list name format string (default: CUSTOMER:%d)
      pfxstr_v6    - The prefix-list name format string (default: CUSTOMERv6:%d)
      pfxlength    - The max length more-specific that will be allowed (default: 24)
      pfxlength_v6 - The max length more-specific that will be allowed for v6 (default: 48)
      format       - The output format for a specific router type (default: cisco)
                     Currently supported values are:
                     cisco
		     ciscoxr
                     extreme
                     foundry
                     force10
                     juniper
                     edgeos
                     huawei

Examples:

    $ bin/irrpt_pfxgen -f cisco 42
    ...
    $ bin/irrpt_pfxgen -f juniper 42
    ...
    $ bin/irrpt_pfxgen -f extreme 42
    ...
    $ bin/irrpt_pfxgen -f force10 42
    ...
    $ bin/irrpt_pfxgen -f edgeos 42
    ...
    $ bin/irrpt_pfxgen -f huawei 42
    ...

In this stage, actual router configurations are generated based on 
the aggregated data we have stored. Currently only the following 
formats are supported, but it should be trivial to add new ones:

1. Cisco/Foundry format (and anyone else with a similar CLI)
2. Juniper format
3. Extreme format
4. Force10 format
5. EdgeOS format (Vyos/Vyatta should also work)
6. Huawei format

These configs can then be deployed automatically using a variety of 
existing tools. Some of these tools include:

* JUNOScript         - http://www.juniper.net/support/junoscript/
* Net::Telnet::Cisco - http://NetTelnetCisco.sourceforge.net/
* RANCID             - http://www.shrubbery.net/rancid/

A few tidbits of examples are included in the "/example" directory.
There are of course a wide variety of ways to deploy configurations 
on routers, and most of them are outside the scope of this project.

Networks may find it appropriate to have an external database in place 
which tracks customer BGP session data, so that you can search by a 
customer name or ASN to automatically find and update only the necessary 
routers and BGP sessions. This is also outside the scope of the current 
project.

Juniper includes an example config pushing script in the JUNOSCript 
package which works well, despite being a little complicated and having 
something like 68 Perl dependancies. 

Cisco can be configured a number of ways, including various perl 
modules, expect scripts. Due to issues like CLI interactivity delays, 
many users find that simply copying config updates via tftp is an easy 
way to manage changes.

The RANCID package includes a few expect scripts for logging in to 
routers to make changes, though they are primarily designed for reading 
status data, not deploying configurations.

Many other systems exist as well.

### irrpt_list_prefixes

Show prefixes for a given AS or AS-SET, in unaggregated or aggregated form.

    $ bin/irrpt_list_prefixes -h
    Usage: bin/irrpt_list_prefixes [-h46va] <object>

Pull unaggregated prefixes:

    $ bin/irrpt_list_prefixes AS-PCH
    2a01:8840:4::/48
    2a01:8840:5::/48
    ...

Pull aggregated prefixes:

    $ bin/irrpt_list_prefixes -a AS-PCH
    2a01:8840:4:2020:2020:2020:2020:2020/47
    ...

Verbose mode:

    $ bin/irrpt_list_prefixes -v -a AS-PCH
    ...
    - Aggregating routes - aggregating neighboring prefixes...
    * WARNING: Aggregating 2a01:8840:0004:0000:0000:0000:0000:0000/48 and 2a01:8840:0005:0000:0000:0000:0000:0000/48 into 2a01:8840:4:2020:2020:2020:2020:2020/47
    ...


### irrpt_list_ases

Show AS numbers for a given AS-SET.

    $ bin/irrpt_list_ases -h
    Usage: bin/irrpt_list_ases [-h46] <object>

Example:

    $ bin/irrpt_list_ases AS-PCH
    AS-PCH
    {
      AS-RS
      {
        AS-CHEREDA-SM
        {
          AS-RS (dup)
          AS197058
          AS21312
    ...


FAQ
---

Q) Wouldn't this be more scalable if we stored our config and data in SQL? 

A) Maybe it would. However, the purpose of this project is to open up the
   world of automated IRR-based prefix-lists to every ISP. As such, it is 
   designed to be as simple as possible, with the fewest complex external 
   dependencies. A well organized network who maintains customer BGP info 
   in an existing database should be able to easily export their database 
   to our config format. It's the disorganized networks we really have to 
   worry about. :)  If you are dealing with so many prefixes that text
   files are too slow, email one of the maintainers.

Q) Does this tool support RPSLng?

A) Not right now. This may change in the future (see the TODO file). Feel 
   free to let me know your strong interest (or lack thereof) on this issue.

Q) Does this tool generate AS-PATH filters?

A) No. Please note that this is not intended as an end-all and be-all 
   RPSL tool, or a total replacement for a utility like IRRToolSet. For 
   example, this tool does not try to parse aut-num import/export policies,
   try to map every relationship between ASNs, try to write your entire 
   route-map for you. This is a pragmatic tool for helping ISPs easily do 
   what 95% of them need to do, namely handle prefixes registered behind a 
   specific IRR aut-num, route-set, or as-set object. Trying to support 
   every possible item which you can document with RPSL is best left to 
   academians and the pedantic.


Long Term Goals
---------------

We understand that this is only one step in the direction of making IRR 
truly and globally useful, but hopefully it will prove to be an important 
one. The next logical step towards improving the overall user experience 
would be to design an intuitive web interface for managing records in the 
IRR, so that BGP speaking end-users are not required to learn the 
complexities of RPSL in order to register their routes.

Hopefully this tool will prove to be useful to a variety of ISPs, and will 
increase the adoption of IRR data over error-prone e-mail updates. When 
more ISPs can make use of this data, and when more end-users are able to 
easily make use of the system, there will hopefully be an increase in 
accurate and maintained data and a reduction in the need to pollute the 
IRR with proxy-registered junk.

More in the TODO file.


Thanks
------

Joe Abley for the very useful aggregate tool which was used in initial 
versions, Chris Morrow for nagging me into releasing this publicly, and all 
the folks in the ChangeLog who helped pick out the bugs. 

A special thanks to Jon Nistor for many many rounds of QA for version 2.0.

Change Log
-------
 
2.1 - Dec 2019 (release pending)

 * Add: pfxgen added support for iosXR
 * Add: pfxgen added support for Ubiquity EdgeOS (@mikenowak)
 * Add: pfxgen added support for openbgpd
 * Add: pfxgen added support for Huawei (@miuvlad)
 * Enh: more help options
 * Enh: AS Object name is now fetched when run via the CLI by specifying a single AS number
 * Fix: undefined offset in aggreagte.inc
 * Fix: cosmetic issue on completed notices
 * Fix: remove Cisco prefix-list associated with proper address family
 * Fix: prefix lengths not properly checked (juniper, iosxr, f10, extreme)
 * Fix: Force10 pfxgen syntax issue
 * Fix: Email notifications comparing full routue and full agg file (@gawul)
 * Fix: iosXR pfxgen generates blank entry for prefixes not matching length (@bonev)
 * Fix: PHP 7.2.0 behaviour change using count(), fixed errors under Juniper pfxgen
 * Fix: Mask php errors for connection timeout, report proper host timeout.
 * Fix: reduced connection timeout to 15s, down from 30s.
 * Fix: typo in iosxr/ciscoxr (@Bierchermuesli)

2.0 - Aug / Sep / Oct 2015

Changes for version 2.0 by Anna Claiborne <domino@theshell.org>:

 * Tagging new version as 2.0
 * Removed system calls from configure.php
 * Updated configure.php print out to be more easily parsible/readable
 * Updated readme docs for version 2.0
 * Created configure.php for initial setup and cvs directory restore
 * Fixed Force10 prefix list syntax in pfxgen
 * Fixed bugs found in running with PHP 7
 * Removed extra and eroneous space from generated emails
 * Fixed irrpt_fetch to check file ownership before attempting permission
   changes
 * Removed calls to system to concatenate v4/v6 route files.  Now performed by 
   php function in utils.inc
 * Provided support to leave email in irrdb.conf blank if the user wishes no 
   email updates for a particular as/object
 * Fixed support for separate (correct) v4 and v6 prefix list for Juniper config.
 * Added AS validation/checking for pfxgen

Changes for version 2.0 by Elisa Jasinska <elisa@bigwaveit.org>:

 * Added v6 support to IRR query, to prefix exclusion via
   exclusions.conf, to aggregation and to the prefix generator
 * Implemented aggregate functionality and removed dependency on the
   external agregate tool
 * -4 and -6 switches for all command line tools
 * Renamed irrpt_eval and irrpt_explorer into irrpt_list_ases
   and irrpt_list_prefixes
 * Added -f option to provide location to irrdb.conf file
 * Added --nocvs option to omit cvs tracking
 * Bug fix for irrpt_list_ases with -6/-4
 * Improved as number vs as string handling in irrpt_fetch
 * Updated config for correct default cvs path
 * Improved ioverall as/as-set input format checking
 * Changed irrpt_list_prefixes to provide aggregated v6 routes in compressed
   form instead of expanded
 * Added command lines options for seperate v4 and v6 perfix list names
 * Added warning when 0 routes found for AS
 * Added v4/v6 command line switches to irrpt_list_ases
 * Better input validation for AS numbers and AS Sets as well as case sensitivity issues 
   resolved

1.28 - June 8 2015

 * Making the latest version compatible with TORIX changes, such as: -q quiet
   mode, timezone support and memory limit support by Elisa Jasinska 
   <elisa@bigwaveit.org>.

1.27 - Feb 8 2008

 * Redesigned (and simplified) the CVS diff code. The options are now
   "fulldiff" (basically the complete output of a normal CVS diff, that
   you could actually use to patch a file), "plusminus" (stripped down
   to only a list of prefix changes), and "english" (aka "Cogent Mode",
   for NOCs who can't quite figure out what the + and - symbols might
   mean). This was supposed to have been done back in 1.20, but this
   version actually makes it work.

1.26 - June 8 2007

 * Added support for Force10 prefix-list format. The style is JUST
   different enough from Cisco to be incompatible, you have to enter a
   hierarchy under the prefix-list declaration and then enter the permit
   lines seperately, rather than stating it all on one line. Thanks to
   Greg Hankins for pointing this out and providing the fix.

 * Added support for 32-bit (4-byte) ASNs. The #.# format (who's stupid
   idea was that anyways :P) was being detected as an IP address and the
   result of a route-set. Added an additional check to make it detect as
   an object to be be further queried, not that it will help since it
   seems the IRRd software (which is the only protocol we support, for
   various reasons) doesn't currently like importing this format either,
   but at least it won't be IRRPT's fault when it doesn't work. :)

   Thanks to: those wonderful euros who just had to actually go and try
   those new ASNs out ASAP, thus killing all the Farce10 and JUNOS 8.3R1
   boxes on the planet. Go go gadget incompatibility. :)

1.25 - May 27 2006

 * Added an option to automatically change uid/gid to a specified id if
   run as root. CVS will not let you run as root, so you should have a
   dedicated user anyways, but sometimes people forget.

 * Completely changed the way we process arguments, php getopt() sucks.

 * Fixed a plethoa of bugs in irrpt_nag. I don't know how you guys were
   even using this before, or why no one told me how broken this was
   before now. :)

 * Changed the default subject line of irrpt_nag, put the ASN info in
   the front for a quicker read on narrow terminals.


1.24 - December 29 2005

 * Added an "ASN list" file which records the ASNs behind each object. This
   may be used to implement some kind of AS-PATH filtering in the future I
   suppose, though it really isn't the right tool for the job. Talked into
   this evilness by Jon Nistor <nistor@snickers.org>.

 * Fixed a really silly mistake in the processing of "english" style output.

 * Added a new tool "irrpt_explorer", which queries and displays the
   contents of an AS-SET in a hierarchal and recursive format.

 * Removed some unnecessary code in irrpt_eval.

1.23 - November 18 2005

 * Changed $_SERVER['SCRIPT_FILENAME'] references to __FILE__ to work around
   some portability issues with certain PHP builds (like apparently, RedHat).
   Pointed out by Joshua Sahala <jejs@sahala.org>.

 * Fixed the -p flag (preview mode) on irrpt_nag. It works better if you
   actually have "p" in getopt(). Pointed out by Christian Malo
   <chris@fiberpimp.net>.

1.22 - November 8 2005

 * Fixed irrpt_eval -a (aggregate) functionality. Pointed out by
   Jon Nistor <nistor@snickers.org>.

 * Added support for ExtremeWare prefix generation. Submitted by Tom
   Hodgson.

 * A documentation tweak noting that PHP >= 4.3 is required. Submitted
   by Tom Hodgson.

 * Added a note regarding Debian's "aggregate" package to the README.

 * Got a little carried away getting rid of Nistor's strict errors. Rolling
   some things back to continue supporting php 4, easier solution is just
   not to turn on E_STRICT.

 * Added some example scripts for router configuration deployment. Updated
   documentation to reflect these changes.

 * Nailed down a few bugs with caching results that had incorrect array
   indexes following sort and unique. Extensive troubleshooting by Jon
   Nistor <nistor@snickers.org>.

 * Added a quick optimization to only parse the exclusions file once (well
   most of the time at any rate).

 * A couple of minor bugfixes and documentation changes from the previous
   release.


1.20 - November 7 2005

 * Added support for handling a route-set object, and revised the as-set code
   to be a little more generic/graceful while handling it. Issue noted by
   Jon Nistor <nistor@snickers.org>.

 * Added a new tool "irrpt_eval" which returns a simple plain-text list of
   routes from an arbitrary IRR object, using the irrpt query engine. This
   can be useful for diagnostics/quick lookups, and is similar to the
   IRRToolSet tool "peval". Requested by Aaron Weintraub.

 * Fixed some class definitions to comply with php strict mode. Contributed
   by Jon Nistor <nistor@snickers.org>.

 * When an IRR query fails, the query that resulted in the failure is now
   displayed (verbose mode required). Nagged to death by Jon Nistor
   <nistor@snickers.org>.

 * Added a config option for controlling the e-mail updates which are sent
   when a route change is detected:

   irrpt.conf: ['fetch']['emailonchange']

   User can now specify whether to send emails for updated detected in the
   "full" (unaggregated) route file, the "aggregated" route file, "both"
   (default), or "none". Requested by Jon Nistor <nistor@snickers.org>

 * Added an option for "english" language diff format. Apparently the Cogent
   NOC can't figure out what "+" and "-" means, so "english" mode changes
   the output to "Add" and "Remove" (and is the new default). There is
   also an option to continue using + and - ("plusminus") but to remove the
   full "diff" headers. The old behavior is retained under the setting
   "fulldiff". Requested by Adam Rothschild.

   irrpt.conf: ['diff']['output_format']

 * Added a new flag "-p" to irrpt_nag, which enables preview mode. In
   preview mode, the contents of the email(s) which would be sent are
   output to stdout instead of being emailed. This allows you to
   double-check the e-mails before actually sending them. Requested by
   Adam Rothschild.

 * Changed the format of ['pfxgen']['default_pfxname'], and renamed it to
   default_pfxstr. It is now handled as a printf()-style format string, with
   one argument (the ASN). For example, to generate "CUSTOMER:1234" use the
   string: "CUSTOMER:%d". Requested by Pierfrancesco Caci.

 * Updated the distributed (but commented out) example exclusions.conf, just
   incase someone decides to uncomment it without obtaining the current
   exclusions list themselves.

 * A bunch of small misc. changes in formatting and alignment. Largely
   requested by Jon Nistor <nistor@snickers.org>


1.10 - December 29 2004

 * Added an optional local cache for prefixes queried from an aut-num
   record, under the assumption that many networks will have as-set
   records which contain overlapping aut-num records. This will increase
   memory usage a bit, but results in significantly faster queries from
   the IRR whois servers for those with a large number of IRRDB entries.
   Cache is enabled by default. Suggested by Arnold Nipper.

 * Fixed a bug in the default CVS files which would cause an error
   message when fetch is run from a directory other than the default
   "/usr/local/irrpt".

 * Commented out the default bogon routes in the distributed exclusions
   config file, so users must choose to explicitly enable it. Hopefully
   this will prevent the blind use of potentially out of date bogon
   information, and avoid unnecessary whining on mailing lists every
   time a RIR is allocated a new /8.


1.00 - December 26 2004

 * Initial public release.
