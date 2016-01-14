#!/usr/bin/perl
use Encode;
use LWP;
use LWP::UserAgent;
use LWP::Simple;


$DEBUG = 0; # set 1 as debug for offline mode
$glb_count = 0;

$DATADIR = "./a22301/"; # root
$DATADIR1 = "./a22301/raw/"; # raw html data
$DATADIR2 = "./a22301/rel/"; # final data


sub do_init_job() {
    printf("init job...\n");
    mkdir($DATADIR);
    mkdir($DATADIR1);
    mkdir($DATADIR2);
}

sub compose_zs_item() {
    my($title,$value,$detail,$rfd) = @_;
    #printf $title;
    #printf $value;
    #printf $detail;
    

    if($title =~ /\<em\>(.*)\<\/em\>/ ){
        printf("$1 \n");
        print($rfd "\"t\":\"". $1 . "\",\n");
    }
    
    if($value =~ /\<span\>(.*)\<\/span\>/) {
        printf("$1\n");
        print($rfd "\"v\":\"". $1 . "\",\n");
    }

    if($detail =~ /\<p\>(.*)\<\/p\>/) {
        printf("$1\n");
        print($rfd "\"d\":\"". $1 . "\"\n");
    }
}

##
# @cityid - a string indicate which city number ID
sub do_parse_zs(){
    my ($cid) = @_;
    my $filename = $DATADIR1 . $cid;
    my $rel_filename = $DATADIR2 . $cid;

    if($DEBUG == 1) {
        $filename = "debug/raw/" . $cid;
        $rel_filename = "debug/rel/" . $cid;
        printf("now, filename=$filename\n");
    }

    my $olderline = "";
    my $title_line = "";
    my $value_line = "";
    my $meet = 0;

    printf("Opening $filename....\n");
    open(FD, "$filename") || die "failed open the raw file($filename)\n";
    
    # create/overwrite the rel processed data file...
    open(RF, ">$rel_filename") || die "failed create the $rel_filename...\n";
    my $rf = \*RF; # passing to sub
    print($rf "\"zs\":{");

    while(<FD>) {
    $curline = $_;
    #print;

    if($meet == 1) {
        # need take this line down
        $glb_count++;

        $elem = "\"z" . $glb_count . "\":{";
        print($rf $elem);

        &compose_zs_item($title_line, $value_line, $_, $rf);
        $meet = 0; # reset

        #FIXME - BUGGY here, how to make last element without ',' symbole?
        if($glb_count == 4) {
            printf($rf "}");
        } else {
            printf($rf "},");
        }
    }
    
    if($curline =~ /em\>紫外线指数/) {
        printf("BINGGO1~~\n");
        $meet = 1;
        #printf("match line:" . $curline);
        #printf("$olderline\n");
        $title_line = $curline;
        $value_line = $olderline;
    }
    
    if ($curline =~ /em\>感冒指数/) {
        printf("BINGGO2~~\n");
        $meet = 1;
        #printf("match line:" . $curline);
        #printf("$olderline\n");
        $title_line = $curline;
        $value_line = $olderline;
    }
    
    if ($curline =~ /em\>洗车指数/) {
        printf("BINGGO3~~\n");
        $meet = 1;
        #printf("match line:" . $curline);
        #printf("$olderline\n");
        $title_line = $curline;
        $value_line = $olderline;
    }
    
    if ($curline =~ /em\>穿衣指数/) {
        printf("BINGGO4~~\n");
        $meet = 1;
        #printf("match line:" . $curline);
        #printf("$olderline\n");
        $title_line = $curline;
        $value_line = $olderline;
    }
#
# as reviewed, below item is not needed [2016-01-07]
#    if ($curline =~ /em\>交通指数/) {
#        printf("BINGGO5~~\n");
#        $meet = 1;
#        #printf("match line:" . $curline);
#        #printf("$olderline\n");
#        $title_line = $curline;
#        $value_line = $olderline;
#    }
    
    #remember
    $olderline = $curline;
    }

    print($rf "}");
    
    close(RF);
    close(FD);
}

sub execute_task() {
    $UA = LWP::UserAgent->new(keep_alive=>1);
    my $retry = 5;
    my ($url, $city_id, $city_name) = @_;

    do {
        $retry --;
        $res = $UA->get($url);

        if($res->is_success) {
            $html = $res->content;
            $fn = $DATADIR1 . $city_id;
            printf("will creating $fn file...\n");
            open(RAWFD, ">", $fn) || die "failed create the content file";
            print(RAWFD  $html);

            close(RAWFD);

            # next will try parse it
            &do_parse_zs($city_id);
            $retry = -1; # don't loop as it successed
        } else {

            if($retry == -1) {
                # this is failed case
                $now = localtime();
                open(LOG, ">>body.txt") || warn "failed open the log file\n";
                print(LOG "  (from Perl) $now failed open the agent($city_id <--> $city_name)\n");
                close(LOG);
            } else {
                # keep next retry
                sleep(2);
            }
        }
    }while($retry >= 0);
}
##########################################################################
### MAIN STARTING POINT

$CITY_ID = $ARGV[0];
$CITY_NAME = $ARGV[1];

$url = "http://www.weather.com.cn/weather1d/" . $CITY_ID . ".shtml";
printf("the city id = $CITY_ID\n");
printf("the whole URL = $url\n");

do_init_job;

if($DEBUG == 1) {
    &do_parse_zs($CITY_ID);
    die "exit for debugging mode...\n";
}

&execute_task($url, $CITY_ID, $CITY_NAME);

