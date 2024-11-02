<?php
/*
Plugin Name: BiblioPress
Plugin URI: http://www.piepalace.ca/blog/projects/bibliopress
Description: Integrate BiblioCommons library data with your blog
Version: 2009-12-29 (Pride)
Author: Erigami Scholey-Fuller
Author URI: http://piepalace.ca/blog/
*/

/*
BiblioPress - BiblioCommons integration with Wordpress (http://wordpress.org).
Copyright (C) 2010  erigami@piepalace.ca

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

// Add hooks for activation and deactivation
register_activation_hook(__FILE__, '_bibliopress_activation');
add_action('bibliopress_change_test', '_bibliopress_update_reviews');

function _bibliopress_activation() {
	wp_schedule_event(time(), 'daily', 'bibliopress_change_test');
}

register_deactivation_hook(__FILE__, '_bibliopress_deactivation');

function _bibliopress_deactivation() {
	wp_clear_scheduled_hook('bibliopress_change_test');
}

add_action('wp_head', '_bibliopress_headers');
add_action('admin_head', '_bibliopress_admin_headers');

/** Do basic init for all pages. ie, include our CSS. */
function _bibliopress_headers() {
    echo '<link type="text/css" rel="stylesheet" href="' . get_bloginfo('wpurl') . '/wp-content/plugins/bibliopress/css/general.css" />' . "\n";
}

function _bibliopress_admin_headers() {
    echo '<link type="text/css" rel="stylesheet" href="' . get_bloginfo('wpurl') . '/wp-content/plugins/bibliopress/css/admin.css" />' . "\n";
}

add_action('admin_menu', '_bibliopress_admin_hook');

/** Shows our stuff on the admin pages. */
function _bibliopress_admin_hook() {
    $optFile = dirname(__FILE__) . '/options.php';
    
    $path = 'bibliopress-opts';
    add_menu_page(__('BiblioPress'), __('BiblioPress'), 'edit_posts', $path, '_bibliopress_option_page', 'div');
    add_submenu_page($path, __('Run BP'), 'Get new items', 'edit_posts', __FILE__, '_bibliopress_update_reviews');
}


/** Helper to handle the usermeta gotchas. */
function _bp_update_usermeta($key, $value) {
    global $user_ID;

    if (get_usermeta($user_ID, $key) == $value) {
        return true;
    }

    return update_usermeta($user_ID, $key, $value);
}

/** Display the user option page. */
function _bibliopress_option_page() {
    global $user_ID;

    $errors = array();
    if ($_POST['bibliopress_action'] == 'save_user_options') {
        $baseurl = trim($_POST['bibliopress_baseurl']);
        if (strlen($baseurl) < 1) {
            $errors[] = "Missing library URL";
        }

        $username = trim($_POST['bibliopress_username']);
        if (strlen($username) < 1) {
            $errors[] = "Missing user name";
        }

        $p1 = $_POST['bibliopress_password_1'];
        $p2 = $_POST['bibliopress_password_2'];
        if ($p1 != $p2) {
            $errors[] = "Passwords do not match";
        }

        if (sizeof($errors) == 0) {
            // Validate url and username/password
            require_once(dirname(__FILE__) . '/bibliocommons.inc');

            $r = BiblioCommons::validate($baseurl, $username, $p1);

            if ($r == BIBLIO_VALID) {
                // Yay! Do nothing.
            } else if ($r == BIBLIO_INVALID_URL) {
                $errors[] = "Library URL is not valid.";
            } else if ($r == BIBLIO_INVALID_USER) {
                if (strlen($p1) > 0) {
                    $errors[] = "Invalid username or password.";
                }
            } else {
                $errors[] = "Unknown return value from validate()";
            }
        }

	    if (sizeof($errors) == 0) {
            // Save the credentials
            get_currentuserinfo();

            $oldBase = get_usermeta($user_ID, 'bibliopress_baseurl');
            if (empty($oldBase)) {
                update_usermeta($user_ID, 'bibliopress_first_time', true);
            }
            
            if (!_bp_update_usermeta('bibliopress_baseurl', $baseurl)) {
                $errors[] = 'Failed to save library URL';
            }

            if (!_bp_update_usermeta('bibliopress_username', $username)) {
                $errors[] = 'Failed to save library user name';
            }

            if (strlen($p1) > 0) {
                if (!_bp_update_usermeta('bibliopress_password', $p1)) {
                    $errors[] = 'Failed to save library password';
                }
            }
        }

	    if (sizeof($errors) == 0) {
	        echo '<div class="updated"><p><strong>' . __('Options saved.', 'BiblioPress') . '</strong></p></div>';
	    } else {
	        echo '<div class="error"><p><strong>';
	        echo __('Options not saved. Correct the following problems: ', 'BiblioPress');
	        foreach ($errors as $e) {
	            echo '<li>';
	            echo $e;
	            echo "</li>\n";
	        }
	        echo '</strong></p></div>';
	    }
    } 

    require(dirname(__FILE__) . '/options.php');
}

/** Get the text of the review. To make life easier, we print it, in the hopes that someone will
 * gobble it up with an output buffer. 
 */
function _bibliopress_print_review($review, $bcid, $title, $item_url, $image_url, $rating, $comment) {
    // See if the user's template knows how to generate a post
    $template = get_template_directory() . '/bibliopress-review.inc';
    if (file_exists($template)) {
        require($template);
        return;
    }

    require(dirname(__FILE__) . '/bibliopress-review.inc');
}

/** Publish a review. We expect a ReviewedLibraryItem. 
 */
function _bibliopress_publish_review($review, $time=null) {
    if (is_null($time)) {
        $time = time();
    }

    // Generate the post

    $bcid = $review->get_bcid();
    $title = $review->get_title();

    $item_url = $review->get_biblio_url();
    $image_url = $review->get_image(200, 200);

    $rating = $review->get_user_rating();
    $comment = $review->get_user_comment();

    ob_start();
    _bibliopress_print_review($review, $bcid, $title, $item_url, $image_url, $rating, $comment);
    $text = ob_get_clean();
    ob_end_clean();

    $post = array(
            'post_date' => date("Y-m-d H:i:s", $time),
            'post_title' => 'Review: ' . $review->get_title(),
            'post_content' => $text,
            'post_status' => 'publish'
    );

    $id = wp_insert_post($post);

    // Add our BiblioCommons metadata
    add_post_meta($id, 'bibliopress_item_bcid', $review->get_bcid());
    add_post_meta($id, 'bibliopress_item_title', $title);
    add_post_meta($id, 'bibliopress_item_url', $review->get_biblio_url());
    add_post_meta($id, 'bibliopress_item_image', $image_url);

    if (!is_null($rating)) {
        add_post_meta($id, 'bibliopress_user_rating', $rating);
    }

    add_post_meta($id, 'bibliopress_user_comment', $review->get_user_comment());
    add_post_meta($id, 'bibliopress_user_review_date', $review->get_review_date());
}

function _bp_log($string) {
    return;
    $log = fopen("/tmp/php.log", "a");
    fwrite($log, $string);
    fwrite($log, "\n");
    fclose($log);
}



/** 
 * Check the reviews for the logged in user.
 */
function _bibliopress_update_reviews() {
_bp_log("Update requested at " . date('r'));
    $users = get_users_of_blog();

    foreach ($users as $user) {
        $userObj = new WP_User($user->ID);
        if ($userObj->has_cap('publish_posts')) {
_bp_log("Querying reviews for: " . $user->user_login . " (" . $user->ID . ')');
            _bibliopress_update_reviews_for_user($user->ID);
        }
    }

_bp_log("Completed update at " . date('r'));
}

function _bibliopress_update_reviews_for_user($id) {
    // Get the time of the last BP post:
    $posts = get_posts('numberposts=1&orderby=date&order=DESC&meta_key=bibliopress_user_review_date&post_status=published');

    $lastPub = 0;
    if (sizeof($posts) == 1) {
        $lastPub = get_post_meta($posts[0]->ID, 'bibliopress_user_review_date', true);
    }

    require_once(dirname(__FILE__) . '/bibliocommons.inc');

    $url = get_usermeta($id, 'bibliopress_baseurl');
    $name = get_usermeta($id, 'bibliopress_username');
    $pass = get_usermeta($id, 'bibliopress_password');

    $f = get_usermeta($id, 'bibliopress_first_time');
    $first = !empty($f);
    unset($f);

    if (is_null($url) || strlen($url) < 1) {
_bp_log("No Bibliopress credentials for: " . $id);
        return;
    }

_bp_log("Querying...");
?>
<h2>Running...</h2>
<?php

    print "Using url: $url <br/>";

    $bc = new BiblioCommons($url, $name, $pass);
    $l = new Library($bc);

    $iter = $l->get_recent_reviews();

    print "Accepting reviews after: " . date("r", $lastPub);
    print '<ol>';

    $pub_time = time();
    foreach ($iter as $review) {
        print '<li/><a href="' . $review->get_biblio_url() . '">'. $review . '</a>';

        if ($lastPub >= $review->get_review_date()) {
            print " AGE BREAK";
            break;
        }

_bp_log("New article for $id: " . $review);
        _bibliopress_publish_review($review, $pub_time--);

        if ($first) {
            print " FIRST TIME BREAK";
            break;
        }

        flush();
    }
    print '</ol>';

    if ($first) {
        delete_usermeta($id, 'bibliopress_first_time');
        print "Removing first-time flag.";
    }
    flush();
}

?>
