#!/usr/bin/perl -w 

use Getopt::Long;
use Data::Dumper;
use File::Path;
use Env;

#-----------------------------------------------------------------------------#
## name of subroutine  : by_nat
## description of func : format convert
## example: 
## foreach my $field (sort by_nat @array){...}
##-----------------------------------------------------------------------------#
sub by_nat {
    ##-----------------------------------------------------------------------------#
    my $a1 = $a;
    my $b1 = $b;

    $a1 =~ s/DsAcl(\d)Ingress/DsAclIngress$1/;
    $a1 =~ s/DsAcl(\d)Egress/DsAclEgress$1/;
    $a1 =~ s/DsAcl(\d)HalfIngress/DsAclHalfIngress$1/;
    $a1 =~ s/DsAcl(\d)HalfEgress/DsAclHalfEgress$1/;

    $b1 =~ s/DsAcl(\d)Ingress/DsAclIngress$1/;
    $b1 =~ s/DsAcl(\d)Egress/DsAclEgress$1/;
    $b1 =~ s/DsAcl(\d)HalfIngress/DsAclHalfIngress$1/;
    $b1 =~ s/DsAcl(\d)HalfEgress/DsAclHalfEgress$1/;


    $a1 =~s/(cfgSgmac)(\d+)(\D.*)/$1$3$2/;
    $b1 =~s/(cfgSgmac)(\d+)(\D.*)/$1$3$2/;

    $a1 =~s/(resetCoreQuadSgmac)(\d+)\b/$1$2/;
    $b1 =~s/(resetCoreQuadSgmac)(\d+)\b/$1$2/;

    $a1 =~s/(resetCoreQuadSgmacReg)(\d+)\b/$1$2/;
    $b1 =~s/(resetCoreQuadSgmacReg)(\d+)\b/$1$2/;

    $a1 =~s/(resetCoreQuadSgmac)(\d+)(Reg)\b/$3$1$2/;
    $b1 =~s/(resetCoreQuadSgmac)(\d+)(Reg)\b/$3$1$2/;
    
     my $aaa = $a1;
     my $bbb = $b1;

    if($a1 =~ /UsxgmiiPcsEeeCtl/)
    {
      $aaa =~ s/\D(\d)$/A$1/g;
      $bbb =~ s/\D(\d)$/A$1/g;

      $aaa =~ s/\D(\d\d)$/B$1/g;
      $bbb =~ s/\D(\d\d)$/B$1/g;

      $aaa =~ s/\D(\d\d\d)$/C$1/g;
      $bbb =~ s/\D(\d\d\d)$/C$1/g;
    }
    else
    {
        my $aa  = $a1;
        my $bb  = $b1;
        $aaa = "_".$a1."_";
        $bbb = "_".$b1."_";

        while($aa =~ /(\d+)/g) {
            my $num =  unpack("B32", pack("N", $1));
            $aaa =~ s/(\D)$1(\D)/$1$num$2/g;
        }
        while($bb =~ /(\d+)/g) {
            my $num =  unpack("B32", pack("N", $1));
            $bbb =~ s/(\D)$1(\D)/$1$num$2/g;
        }
    }

    $aaa cmp $bbb;
}


$curdir = `pwd`;
$runner = `whoami`;
chomp $runner;
chomp $curdir;

# DRV_DEF_D(Aneth, 40, AnethAdvAbility1, SLICE_Default, Op_Default | Op_DisBurst | Op_DisWriteMask, 1, 7, 0x00140b00)
# DRV_DEF_F(Aneth, 40, AnethAdvAbility1, cfgAdvAbility, 48, {1,0,16}, {0,0,32})

$format1 = 'DRV_DEF_D';
$format2 = 'DRV_DEF_F';

$ndb = {};

push @files,  "drv_ds_dt2.h";
$dt2='dt2';
push @files,  "drv_ds_tm.h";
$tm='tm';

foreach $file (@files) {

#    print "file = $file\n";
#    print "---------------------------------------------\n";
    open (TXT, "$file") || die "unable to open $file\n";

    while(<TXT>) {
        chomp;
        if($_ =~ m/$format1/) {
            @args =  split /,/, $_;
            $r = $args[2];
            $w = $args[6];
            $r =~ s/^\s+|\s+$//;
            $w =~ s/^\s+|\s+$//;
            #print "r = $r w = $w\n";

            if(exists($ndb->{$r}{word})) {

                if($w > $ndb->{$r}{word}) {
                  $ndb->{$r}{word} = $w;
                }

            } else {
                $ndb->{$r}{word} = $w;
            }

        } elsif ($_ =~ m/$format2/) {
            @args =  split /,/, $_;
            $r = $args[2];
            $f = $args[3];
            $f1 = $args[3];
            $r =~ s/^\s+|\s+$//;
            $f =~ s/^\s+|\s+$//;
            $f1 =~ s/^\s+|\s+$//;
            #print "r = $r f = $f\n";
            $ndb->{$r}{field}{$f}{value} = $f1;
        } else {
        }
    }
    close TXT
}

foreach $file (@files) {

#    print "file = $file\n";
#    print "---------------------------------------------\n";
        if($file =~ m/$dt2\b/) {
            $chip = "DT2";
        }

        if($file =~ m/$tm\b/) {
            $chip = "TM";
        }


    open (TXT, "$file") || die "unable to open $file\n";

    while(<TXT>) {
        chomp;
        if($_ =~ m/$format1/) {
            @args =  split /,/, $_;
            $r = $args[2];
            $w = $args[6];
            $r =~ s/^\s+|\s+$//;
            $w =~ s/^\s+|\s+$//;
            #print "r = $r w = $w\n";
            push @{$ndb->{$r}{chip}}, $chip;

        } elsif ($_ =~ m/$format2/) {
            @args =  split /,/, $_;
            $r = $args[2];
            $f = $args[3];
            $r =~ s/^\s+|\s+$//;
            $f =~ s/^\s+|\s+$//;
            #print "r = $r f = $f\n";
            push @{$ndb->{$r}{field}{$f}{chip}}, $chip;
        } else {
        }
    }
    close TXT
}

my $i = 0;
my $j = 0;

foreach my $r (sort by_nat keys %{$ndb}) {
    if ($#{$ndb->{$r}{chip}} == $#files) {
        print "DRV_DEF_D($r, $ndb->{$r}{word}, $i)\n";
    } else {
        print "DRV_DEF_D($r, $ndb->{$r}{word}, $i) /*";
        foreach $c (@{$ndb->{$r}{chip}})
        {
            print "$c.";
        }
        print "*/\n";
    }
    $i++;

    $j=0;
    foreach my $f (sort by_nat keys %{$ndb->{$r}{field}}) {

        $f1 = $ndb->{$r}{field}{$f}{value};
        if ($#{$ndb->{$r}{field}{$f}{chip}} == $#files) {
            print "DRV_DEF_F($r, $f1, $j)\n";
        } else {
            print "DRV_DEF_F($r, $f1, $j) /*";
            foreach $c (@{$ndb->{$r}{field}{$f}{chip}})
            {
                print "$c.";
            }
            print "*/\n";
        }
        $j++;
    }


#    print "\n"
}

