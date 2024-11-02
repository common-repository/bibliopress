<?php
    // Generate a basic review
    print '<div class="bibliopress_review">';

    // We wrap these sections in divs to prevent added whitespace from WP
    print '<div class="bibliopress_item_image"><a href="' . $item_url . '">';
    print '<img src="' . $image_url . '" align="right"/></a></div>';

    print '<div class="bibliopress_meta bibliopress_item">';
    print '<a href="' . $item_url . '" class="bibliopress_item_title">'; 
    print htmlentities($title);
    print '</a>';

    // Add a by-line
    $authors = $review->get_authors();
    if (!empty($authors)) {
        print ' by ';
        
        for ($i = sizeof($authors) - 1; $i >= 0; $i--) {
            print $authors[$i];

            if ($i > 1) {
                print ', ';
            } elseif ($i == 1) {
                print ' and ';
            } 
        }

        print '.';
    }

    print '</div>';

    if (!is_null($rating)) { 
        print '<div class="bibliopress_meta bibliopress_user_rating" title="' . $review->get_user_rating_string() . '">';

        for ($i = 2; $i <= 10; $i += 2) {
            $mult = $rating * 10;

            $star = null;
            if ($mult > $i - 2) {
                if ($mult > $i - 1) {
                    $star = 'star.png';
                } else {
                    $star = 'star-desat-half.png';
                }
            } else {
                $star = 'star-desat.png';
            }

            echo '<img src="' . get_bloginfo('wpurl') . '/wp-content/plugins/bibliopress/images/' . $star . '" />';
        }
        print "</div>\n";
    }

    print '<div class="bibliopress_user_comment">';
    print $comment;
    print '</div></div>';
?>
