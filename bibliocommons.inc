<?php

// FILE: Interface to bibliocommons. Waka waka!

/** Indicates that we are doing an HTTP 'GET'. */
define('GET', 'GET');

/** The logical type of a community credit. */
define('BIBLIO_CC_TYPE_COMMENT', 'biblio_cc_comment');
define('BIBLIO_CC_TYPE_TAG', 'biblio_cc_tag');
define('BIBLIO_CC_TYPE_SIMILAR', 'biblio_cc_similar');
define('BIBLIO_CC_TYPE_RATING', 'biblio_cc_rating');

define('BIBLIO_INVALID_URL', 'biblio_bad_url');
define('BIBLIO_INVALID_USER', 'biblio_bad_user');
define('BIBLIO_VALID', 'biblio_ok');

/** Attach to a library and do stuff. */
class BiblioCommons {

    /** Verify that the url/username/password tuple is valid. 
     * 
     * Returns:
     * - BIBLIO_INVALID_URL if the response doesn't look like a bibliocommons response.
     * - BIBLIO_INVALID_USER if the url was valid, but login failed. 
     * - BIBLIO_VALID if the tuple is good.
     * 
     * Note that checks are made in the above order. If BIBLIO_INVALID_USER is 
     * returned, then the URL is valid.
     */
    public static function validate($url, $user, $pass) {
        $tester = new BiblioCommons($url, $user, $pass);

        # Verify the URL
        try {
            $obj = $tester->request(GET, "Libraries", false);
        } catch (Exception $e) {
            return BIBLIO_INVALID_URL;
        }

        # Verify the username and password
        try {
            if (!$tester->auth()) {
                return BIBLIO_INVALID_USER;
            }
        } catch (Exception $e) {
            return BIBLIO_INVALID_USER;
        }

        return BIBLIO_VALID;
    }

    /** The auth token returned by BC. */
    private $session_id = null;

    /** The name to use when logging in. */
    private $name = null;

    /** The passward to use when logging in. */
    private $pass = null;

    /** Base URL for requests. */
    private $URL;

    /** Base URL for items (ie, human consumable item pages) */
    private $item_url;

    /** The ID of the library. */
    private $library_id = "14";

    public function __construct($url, $name, $pass) {
        $sanitizedUrl = $this->sanitize_url($url);

        $this->URL = $sanitizedUrl . '/api/';
        $this->item_url = $sanitizedUrl . '/item/show/{bcid}';

        $this->name = $name;
        $this->pass = $pass;
    }

    private function sanitize_url($url) {
        $url = trim($url);
        
        while (strrpos($url, '/') == strlen($url) - 1) {
            $url = substr($url, 0, -1);
        }

        return $url;
    }

    /** Authenticate, if we haven't already authenticated. */
    private function auth() {
        if (!is_null($this->session_id)) {
            return;
        }

        $xml = $this->request(GET, 'Library/{library_id}/logIn/' . $this->name . '/' . $this->pass, false);

        $sessionId = (string)$xml->SessionId;
        if (strlen($sessionId) == 0) {
            return false;
        }

        $this->session_id = $sessionId;

        return true;
    }

    /** Substitute variables from the path into the given URL path.*/
    private function path_transformed($r) {
        $r = str_replace('{library_id}', $this->library_id, $r);
        $r = str_replace('{session_id}', $this->session_id, $r);

        return $r;
    }

    public function request($method, $path, $authenticate = true, $iterable_tag = false) {
        if ($authenticate) {
            $this->auth();
        }

        $raw_path = $this->path_transformed($path);
#print "URL: " . $this->URL . $raw_path . "\n";

        // Make the request
        $ch = curl_init($this->URL . $raw_path);

        curl_setopt($ch, CURLOPT_HEADER, 0);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true); 

        $text = curl_exec($ch);
        $errno = curl_errno($ch);
        $error = curl_error($ch);
        curl_close($ch);

        if ($errno != 0) {
            throw new Exception("Nonzero return from cURL: " . $errno . " (" . $error . ')');
        }

#print "TEXT: ";
#print $text;
#print "\n";
        // Decode the request
        $xml = @new SimpleXMLElement($text);

        if ($iterable_tag) {
            $xml = new BiblioCommons_iterator($this, $raw_path, $xml, $iterable_tag);
        }

        return $xml;
    }

    /** Generate an URL for the current library. */
    function make_item_url($bcid) {
        return str_replace('{bcid}', $bcid, $this->item_url);
    }

    function load_next_page($url, $page_index, $page_size) {
        return $this->request(GET, $url . "/SetPageSize/$page_size/SetPage/$page_index/", true);
    }

    /** Return a listing of known libraries. */
    function get_libraries() {
        return $this->request(GET, 'Libraries', false);
    }

    /** Get an iterator that can walk over the user's community credits. */
    function get_user_community_credits() {
        return $this->request(GET, 'LoggedInAccount/{session_id}/User/CommunityCredits', true, 'CommunityCredit');
    }

    function get_image_url($bcid, $x = null, $y = null) {
        if ($x == null && $y == null) {
            return $this->request(GET, 'ContentService/DefaultImageUrl/' . $bcid, false);
        } else {
            return $this->request(GET, 'ContentService/ImageUrl/' . $bcid . '/' . $x . '/' . $y, false);
        }
    }
}


/** Provides an iteration interface over BC objects. 
 * TODO: Handle paging. =)
 */
class BiblioCommons_iterator implements Iterator {
    private $url = null;
    private $first_page = null;
    private $page = null;
    private $value_tag = null;

    /** The logical index into the array. */
    private $i = 0;

    public function __construct($bc, $url, $page, $value_tag) {
        $this->bc = $bc;
        $this->url = $url;
        $this->first_page = $page;
        $this->page = $page;
        $this->value_tag = $value_tag;
    }

    public function current() {
        $var = $this->value_tag;

        // Do we need to load the next page?
        if (($this->page->PageSize * (1 + $this->page->Page)) <= $this->i) {
            $this->page = $this->bc->load_next_page($this->url, $this->page->Page + 1, $this->page->PageSize);
        }

        $page_offset = $this->page->PageSize * $this->page->Page;

        $arr = $this->page->$var;
        return $arr[$this->i - $page_offset];
    }

    public function key() {
        return $this->i;
    }

    public function next() {
        $this->i ++;
    }

    public function rewind() {
        $this->i = 0;
        $this->page = $this->first_page;
    }

    public function valid() {
#print "valid: " . $this->i . " total: " . $this->page->TotalCount. "\n";
        return $this->i < $this->page->TotalCount;
    }
}

/** Provide a version-independent (we hope) view of BiblioCommons data. */
class BiblioBuddy {

    /** Returns a logical rating object for a community credit. */
    public static function get_review_type_for($community_credit) {
        $mapping = array(
            'TAG' => BIBLIO_CC_TYPE_TAG,
            'COMMENT' => BIBLIO_CC_TYPE_COMMENT,
            'RATING' => BIBLIO_CC_TYPE_RATING,
            'SUGGESTION' => BIBLIO_CC_TYPE_SIMILAR
        );

        return $mapping[(string)$community_credit->Type];
    }

}

class Library {
    public function __construct($bc) {
        $this->bc = $bc;
    }

    /** Return an iterator for the items recently reviewed. 
     */
    public function get_recent_reviews() {
        $creds = $this->bc->get_user_community_credits();

        return new RecentReviewIterator($this->bc, $creds);
    }
}


/** An iterator that walks a bunch of community credits and 
 * provides a new item for each COMMENT review.
 */
class RecentReviewIterator implements Iterator {
    private $bc;
    private $credits;

    /** The logical index into the array. */
    private $i = 0;

    /** A list of visited BibIds */
    private $seen = array();

    private $current_review = null;

    public function __construct($bc, $credits) {
        $this->bc = $bc;
        $this->credits = $credits;
    }

    public function current() {
        return $this->current_review;
    }

    public function key() {
        return $this->i;
    }

    public function next() {
        $this->i++;
        $this->credits->next();
    }

    public function rewind() {
        $this->credits->rewind();
        $this->i = 0;
    }

    public function valid() {
        while (true) {
            if (!$this->credits->valid()) {
                return false;
            }

            // Filter down to event types we care about
            $cred = $this->credits->current();

#print (string)$cred->Bib->Title . "\n";
            if (BIBLIO_CC_TYPE_COMMENT == BiblioBuddy::get_review_type_for($cred)) {
                $this->current_review = new ReviewedLibraryItem($this->bc, $cred);
                return true;
            }

            // Walk forward to the next unseen id
            $this->credits->next();
        }
    }
}

/** An item in a BC library. */
class LibraryItem {

    public function __construct($bc, $item) {
        $this->bc = $bc;
        $this->item = $item;
    }

    /** Get the title for the given item. */
    public function get_title() {
        return (string)$this->item->Title;
    }

    /** Get the URL of the item in BiblioCommons. */
    public function get_biblio_url() {
        return $this->bc->make_item_url((string)$this->item->BcId);
    }

    /** Get the image of the receiver. */
    public function get_image($x = null, $y = null) {
        return $this->bc->get_image_url((string)$this->item->BcId, $x, $y);
    }

    /** Get the bcid of the item. */
    public function get_bcid() {
        return $this->item->BcId;
    }

    private static function make_author_human_readable($author_string) {
        $split = explode(',', $author_string);

        if (sizeof($split) == 1) {
            return trim($split[0]);
        }
        else if (sizeof($split) == 2) {
            return trim($split[1]) . ' ' . trim($split[0]);
        }
        
        return trim($author_string);
    }

    /** Return an array of strings, naming the author(s). */
    public function get_authors() {
        $authors = $this->item->Authors;

        $r = array();
        if (sizeof($authors->children()) == 0) {
            $authors = $this->item->AddedAuthors->String;
            
            foreach ($authors as $author) {
                $r[] = LibraryItem::make_author_human_readable((string)$author);
            }
        } else {
            foreach ($authors as $author) {
                $r[] = LibraryItem::make_author_human_readable((string)($author->String));
            }
        }

        return $r;
    }

    public function __tostring() {
        return "{LibraryItem " . $this->get_title() . "}";
    }
}

class ReviewedLibraryItem extends LibraryItem {
    private $credit;

    public function __construct($bc, $credit) {
        parent::__construct($bc, $credit->Bib);
        $this->credit = $credit;
#print_r($credit);
    }

    /** Return the rating given by the user. May be 'null' if it isn't set. */
    public function get_user_rating() {
        $r = (string)$this->credit->UserContent->Rating;

        if ($r == '0') {
            return null;
        }

        return ((float)$r) / 10.0;
    }

    /** Get a textual version of the rating given by the user */
    public function get_user_rating_string() {
        $r = (string)$this->credit->UserContent->Rating;

        if ($r == '0') {
            return "";
        }

        switch ($r) {
            case(0): return "";
            case(1): return "Awful";
            case(2): return "Poor";
            case(3): return "Not bad";
            case(4): return "Ok";
            case(5): return "Average";
            case(6): return "Above average";
            case(7): return "Good";
            case(8): return "Very good";
            case(9): return "Great";
            case(10): return "";
        }

        return "";
    }

    /** Return the comment given to this item by the user. May be 'null' if it isn't set. */
    public function get_user_comment() {
        return $this->credit->UserContent->Comment;
    }

    /** Return a *nix timestamp of the date the review was made. */
    public function get_review_date() {
        // Welcome to stupid land. A land where nonstandard date formats and wonky PHP date 
        // funcations make everything suck. 
        $date = $this->credit->UserContent->CommentDate;
        $date = substr($date, 0, 19) . substr($date, 23);
        $time = strptime($date, "%Y-%m-%d %H:%M:%S %z");

        $tstamp = mktime($time["tm_hour"], $time["tm_min"], $time["tm_sec"], $time["tm_mon"] + 1, $time["tm_mday"], $time["tm_year"] + 1900);

        return $tstamp;
    }

    public function __tostring() {
        $rat = $this->get_user_rating();
        if (is_null($rat)) {
            $rat == '';
        } else {
            $rat = ' rating: ' . $rat;
        }

        return '{ReviewedLibraryItem title: "' 
            . $this->get_title() 
            . '" date: "'
            . date("r", $this->get_review_date())
            . '"'
            . $rat 
            . '}';
    }
}

#$bc = new BiblioCommons("http://bibliocommons.biblioottawalibrary.ca/", 'erigami', 'xxxx');
#$l = new Library($bc);
#
#$iter = $l->get_recent_reviews();
#
#$ct = 5;
#foreach ($iter as $review) {
#    print "-----------------------------------\n";
#    print "Title: " . $review->get_title() . "\n";
#    print "URL: " . $review->get_biblio_url() . "\n";
#    print "Creds: ";
#    print_r($review->get_authors());
#    print_r($review);
#    if ($ct-- <= 0) {
#        break;
#    }
#    #print "Image: " . $review->get_image(200, 200) . "\n";
#    #$date = $review->get_review_date() ;
#    #print "Date: " . $date . " (" . date("%r", $date) . ")\n";
#    #$r = $review->get_user_rating();
#    #print "Rating: >" . $r . "< is_null: " . is_null($r) . "\n\n\n";
#    #$c = $review->get_user_comment();
#    #print "Comment: is_null: " . is_null($c) . " " . $c . "\n";
#}
#
##print BiblioCommons::validate("http://bibliocommons.biblioottawalibrary.ca/", 'a', 'b');
?>
