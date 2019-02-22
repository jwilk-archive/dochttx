#!/usr/bin/env perl

# Copyright © 2019 Jakub Wilk <jwilk@jwilk.net>
# SPDX-License-Identifier: MIT

no lib '.';  # CVE-2016-1238

use strict;
use warnings;

use English qw(-no_match_vars);
use FindBin ();
use Test::More tests => 6;

use IPC::System::Simple qw(capture);

my $basedir = "$FindBin::Bin/..";
my $target = $ENV{DOCHTTX_TEST_TARGET} // "$FindBin::Bin/../dochttx";

my $cli_version_re = qr/\Adochttx (\S+)\n/;
my $stdout = capture($target, '--version');
like($stdout, $cli_version_re , 'well-formed version information');
my ($cli_version) = $stdout =~ $cli_version_re;

my $changelog_version_re = qr/\Adochttx [(](\S+)[)] (\S+); urgency=\S+\n\z/;
open(my $fh, '<', "$basedir/doc/changelog") or die $ERRNO;
my $changelog = <$fh> // die $ERRNO;
close($fh) or die $ERRNO;
like($changelog, $changelog_version_re, 'well-formed changelog title line');
my ($changelog_version, $distribution) = $changelog =~ $changelog_version_re;
SKIP: {
    if (-d "$basedir/.hg") {
        skip('hg checkout', 1);
    }
    if (-d "$basedir/.git") {
        skip('git checkout', 1);
    }
    cmp_ok(
        $distribution,
        'ne',
        'UNRELEASED',
        'distribution != UNRELEASED',
    );
}
cmp_ok(
    ($cli_version // ''),
    'eq',
    ($changelog_version // ''),
    'CLI version == changelog version'
);

my $man_version_re = qr/^[.]TH DOCHTTX 1 \S+ "dochttx (\S+)"/;
open($fh, '<', "$basedir/doc/dochttx.1") or die $ERRNO;
my $th = '';
while (<$fh>) {
    if (m/^[.]TH /) {
        $th = $_;
        last;
    }
}
close($fh) or die $ERRNO;
like($th, $man_version_re, 'well-formed TH command');
my ($man_version) = $th =~ $man_version_re;
cmp_ok(
    ($cli_version // ''),
    'eq',
    ($man_version // ''),
    'CLI version == man page version'
);

# vim:ts=4 sts=4 sw=4 et
