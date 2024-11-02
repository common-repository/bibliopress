<?php

/** Show the claim management page */
global $wpdb;
?>
<div class="wrap">
<div id="bibliopress-icon-32"><h2>BiblioPress User Settings</h2></div>

<form method="post" action="">
<?php 

wp_nonce_field('update-options'); 

// Get the user meta to show
global $user_ID;
get_currentuserinfo();

$url = htmlentities(get_usermeta($user_ID, 'bibliopress_baseurl'));
$username = htmlentities(get_usermeta($user_ID, 'bibliopress_username'));

?>
<input type="hidden" name="bibliopress_action" value="save_user_options"/>


<table class="form-table">
    <tr valign="top">
        <th scope="row">Library URL</th>
        <td>

            <label><input name="bibliopress_baseurl" type="text" id="bibliopress_baseurl" class="regular-text code" value="<?php echo $url; ?>"/>
                <span class="description">Website address of your library's catalog.</span>
            </label>
        </td>
    </tr>

    <tr valign="top">
        <th scope="row">Library Username</th>
        <td>
            <input name="bibliopress_username" type="text" id="bibliopress_username" value="<?php echo $username; ?>"/>
        </td>
    </tr>

    <tr valign="top">
        <th scope="row">Library Password</th>
        <td>
            <input name="bibliopress_password_1" type="password"/>
            <span class="description">Password used to log into the library catalogue.</span>
        </td>
    </tr>

    <tr valign="top">
        <th scope="row">Confirm Password</th>
        <td>
            <input name="bibliopress_password_2" type="password"/>
        </td>
    </tr>
<!--
    <tr valign="bottom">
        <th scope="row">Item Reviews</th>
        <td>

            <label><input name="bibliopress_post_reviews" type="checkbox" id="bibliopress_post_reviews"/>
                Post your reviews on your blog.
            </label>
        </td>
    </tr>
-->

</table>

<p class="submit">
    <input class="button-primary" type="submit" name="Submit" value="<?php _e('Save Changes') ?>" />
</p>
</form>

