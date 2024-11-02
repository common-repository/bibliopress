=== Plugin Name ===
Contributors: erigami
Tags: library, integration, Bibliocommons, review, publish, post, book, movie
Tested up to: 2.8
Requires at least: 2.8
Stable tag: 2009-12-28

Publish book and movie reviews from your local library. Automatically posts your reviews with the rating and cover art.

== Description ==

BiblioPress automatically publishes your reviews from libraries running Bibliocommons. The plugin runs daily, gathering your 
new reviews and ratings, and posts them onto your blog. The posts include your rating as well as the cover art for the book or 
movie reviewed.

== Installation ==

There are two methods to install BiblioPress to your WordPress installation. You can either uncompress the zip file and upload it 
to the `wp-content/plugins/`, or you can install it using the _Plugins_/_Add New_ menu. 

After installing, add your Bibliocommons credentials to the BiblioPress settings page. You can find detailed instructions at 
the author's [plugin page](http://www.piepalace.ca/blog/projects/bibliopress).

== Screenshots ==

1. Sample review. Note that the appearance of your reviews will depend on your theme. 
2. BiblioPress configuration page. Each user has their own configuration page, and can update the stuff by themselves.

== Changelog ==

= 2009-12-29 (Pride) =
- Added author/director to output
- Broke review text out into separate file

= 2009-12-28 (Shame) =

- Fixed error in validation: passing incorrect arguments to Bibliocommons object
- Made update page visible to non-admin users
- Added icon for settings page

= 2009-12-27 (Denial) =

- Initial revision
- Support: setting username/password/library-uri, reposting reviews
- Override review template with file in theme
- Improved option page: validation of library uri, validation of username/password
- Improved initial installation experience: only publish the most recent review

__Note:__ Contributions are welcome. Email <erigami@piepalace.ca> to help out. 

= Planned Release: Acceptance =

- Add paging control the Bibliocommons adapter, and force the standard page size to 100 items
- Improved UI for running a scrape
- Put BiblioPress menu items in more appropriate location


= Planned Release: Questioning =

- Association of category/tags with new reviews
- Added history applet to dashboard 
- Added error banner to dashboard page (on case of failed login)


= Unscheduled =

I have no plans to implement the following features. 

- Change the prefix of the review


== Upgrade Notice ==

= 2009-12-29 (Pride) = 
This release contains functional improvements and bugfixes. It improves functionality.

= 2009-12-28 (Shame) = 
This release contains functional improvements and bugfixes. It improves functionality.

= 2009-12-27 (Denial) =
This version is the initial release. 

== Frequently Asked Questions ==

To ask a question, see the author's [plugin page](http://www.piepalace.ca/blog/projects/bibliopress).

= How do I know if my library is a Bibliocommons library? =

Go to your library's catalogue, and scroll down to the bottom of the page. If your library's catalogue is managed by Bibliocommons, 
there should be a line of text similar to _Powered by BiblioCommons, with the financial support of Knowledge Ontario._ 
