#!/usr/bin/env perl

# Copyright © 2019 Jakub Wilk <jwilk@jwilk.net>
# SPDX-License-Identifier: MIT

no lib '.';  # CVE-2016-1238

use strict;
use warnings;

use File::Temp ();
use FindBin ();
use Test::More tests => 1;

use IPC::System::Simple qw(capture);

my $target = $ENV{DOCHTTX_TEST_TARGET} // "$FindBin::Bin/../dochttx";

my $tmpdir = File::Temp->newdir(TEMPLATE => 'dochttx.test.XXXXXX', TMPDIR => 1);

sub run_tmux
{
    my @args = @_;
    return capture(qw(tmux -f /dev/null -S), "$tmpdir/tmux.socket", @args);
}

my $tmux_version = run_tmux('-V');
diag($tmux_version);
run_tmux('new-session', '-d', '--', "$target -d /dev/null");
sleep(1);
my $out = run_tmux('capture-pane', '-ep');
run_tmux('kill-session');
cmp_ok($out, 'ne', '', 'screen capture');
diag($out);

# vim:ts=4 sts=4 sw=4 et
