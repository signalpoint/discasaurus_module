<?php
/**
 * @file
 * Contains \Drupal\disc\Controller\DiscController.
 */
namespace Drupal\disc\Controller;
class DiscController {
    public function content() {
        return array(
            '#type' => 'markup',
            '#markup' => t('Hello, World!'),
        );
    }
}