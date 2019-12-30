  <?php
// $Id: page.tpl.php,v 1.4.2.6 2011/02/18 05:26:30 andregriffin Exp $
?>
<?php
// page.tpl.php
//if ($is_front):
  //include('page--front.tpl.php');
  //return;
//endif;
?>
<?php if ($page['masthead']): ?>
  <header id="masthead" class="clearfix">
  	<div id="masthead-inner">
		<?php print render($page['masthead']); ?>
    </div>
	<script>
  (function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){
  (i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),
  m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)
  })(window,document,'script','//www.google-analytics.com/analytics.js','ga');

  ga('create', 'UA-43404603-3', 'auto');
  ga('send', 'pageview');

</script>
  </header>
<?php endif; ?>
<div id="wrapper" class="clearfix">

  <div id="skip-link">
    <a href="#main-content" class="element-invisible element-focusable"><?php print t('Skip to main content'); ?></a>
    <?php if ($main_menu): ?>
      <a href="#navigation" class="element-invisible element-focusable"><?php print t('Skip to navigation'); ?></a>
    <?php endif; ?>
  </div>

    
  
  <header id="header" role="banner" class="clearfix">
	<div id="logo-block">
		<?php if ($logo): ?>
          <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home" id="logo">
            <img src="<?php print $logo; ?>" alt="<?php print t('Home'); ?>" />
          </a>
        <?php endif; ?>
    </div>
    <?php if ($site_name || $site_slogan): ?>
      <div id="site-name-slogan">
        <?php if ($site_name): ?>
          <?php if ($title): ?>
            <div id="site-name"><strong>
              <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home"><span><?php print $site_name; ?></span></a>
            </strong></div>
          <?php else: /* Use h1 when the content title is empty */ ?>
            <h1 id="site-name">
              <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home"><span><?php print $site_name; ?></span></a>
            </h1>
          <?php endif; ?>
        <?php endif; ?>
        <?php if ($site_slogan): ?>
          <div id="site-slogan"><?php print $site_slogan; ?></div>
        <?php endif; ?>
      </div>
    <?php endif; ?>
    <div id="header-blocks"><?php print render($page['header']); ?></div>
  </header> <!-- /#header -->

  <?php if ($main_menu || $secondary_menu): ?>
    <nav id="navigation" role="navigation" class="clearfix">
      <?php if ($page['navigation']): ?> <!--if block in navigation region, override $main_menu and $secondary_menu-->
        <?php print render($page['navigation']); ?>
      <?php endif; ?>
      <?php if (!$page['navigation']): ?>
        <?php if ($main_menu): print $main_menu; endif; ?>
        <?php if ($secondary_menu): print $secondary_menu; endif; ?>
      <?php endif; ?>
    </nav> <!-- /#navigation -->
  <?php endif; ?>
  
  <section id="content-header">
  	<section id="content-header-inner" class="row">
    	<section id="content-header-sub-inner">
            <?php print render($page['help']); ?>
            <section id="messages" role="messages" class="clearfix">
				<?php print $messages; ?>
            </section>
        </section>
   	</section>
   </section>
  
  <section id="frontpage_outer">
    <section id="main" role="main" class="clearfix">
	  <?php if ($breadcrumb): ?>
              <?php print $breadcrumb;?>
      <?php endif; ?>
      <section id="content-region" class="content-region row nested white-bg">
          <a id="main-content"></a>
          <?php if ($page['highlighted']): ?><div id="highlighted"><?php print render($page['highlighted']); ?></div><?php endif; ?>
          <?php print render($title_prefix); ?>
          <?php if ($title): ?><h1 class="title" id="page-title"><?php print $title; ?></h1><?php endif; ?>
          <?php print render($title_suffix); ?>
          <?php if (!empty($tabs['#primary'])): ?><div class="tabs-wrapper"><?php print render($tabs); ?></div><?php endif; ?>
          <?php if ($action_links): ?><ul class="action-links"><?php print render($action_links); ?></ul><?php endif; ?>
          <?php print render($page['content']); ?>
      </section>  </section> <!-- /#main -->
    <!-- /#footer -->
    <footer id="footer" role="contentinfo">
    <?php print render($page['footer']); ?>     
    </footer><!-- /#footer --> 
  </section>
  
  <?php if ($page['sidebar_first']): ?>
    <aside id="sidebar-first" role="complimentary" class="sidebar clearfix">
      <?php print render($page['sidebar_first']); ?>
    </aside>  <!-- /#sidebar-first -->
  <?php endif; ?>

  <?php if ($page['sidebar_second']): ?>
    <aside id="sidebar-second" role="complimentary" class="sidebar clearfix">
      <?php print render($page['sidebar_second']); ?>
    </aside>  <!-- /#sidebar-second -->
  <?php endif; ?>

</div> <!-- /#wrapper -->