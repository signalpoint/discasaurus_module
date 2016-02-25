<?php

/**
 * @file
 * Contains \Drupal\disc\Controller\DiscBatchController.
 */

namespace Drupal\disc\Controller;

use \Drupal\Core\Link;
use \Drupal\Core\Url;

class DiscBatchController {
  public function content() {

    $type = isset($_GET['type']) ? $_GET['type'] : NULL;
    $operations = [];

    $limit = 72;

    switch ($type) {

      // Courses.
      case 'dg_course':

        // Load all the courses.
        disc_switch_db(6);
        $result = db_select('node', 'n')
          ->fields('n', array('nid', 'title'))
          ->condition('n.type', $type)
          ->orderBy('n.nid', 'ASC')
          ->range(0, $limit)
          ->execute();
        $courses = $result->fetchAll();
        disc_switch_db(8);

        // Build an operation for each course.
        foreach ($courses as $course) {
          $args = array(
            'nid' => $course->nid,
            'title' => $course->title
          );
          $operations[] = array('disc_migrate', array($type, $args));
        }

        break;

      // Holes.
      case 'dg_hole':

        // Determine how many holes there are.
        disc_switch_db(6);
        $holeCount = db_select('node')
          ->fields(NULL, array('nid'))
          ->condition('type', $type)
          ->countQuery()
          ->execute()
          ->fetchField();
        disc_switch_db(8);
        //dpm($holeCount);

        // Set up paging.
        $pageSize = $limit;
        $pages = ceil($holeCount / $pageSize);
        $page = isset($_GET['page']) ? $_GET['page'] : 0;
        dpm('pageSize: ' . $pageSize);
        dpm('pages: ' . $pages);
        dpm('page: ' . $page);

        //dpm($results->fetchassoc());

        // Load a page of holes.
        disc_switch_db(6);
        $result = db_select('node', 'n')
          ->fields('n', array('nid'))
          ->condition('n.type', $type)
          ->orderBy('n.nid', 'ASC')
          ->range($page * $pageSize, $pageSize)
          ->execute();
        $holes = $result->fetchAll();
        disc_switch_db(8);

        // Build an operation for each hole.
        foreach ($holes as $hole) {
          $args = array(
            'nid' => $hole->nid,
            'page' => $page
          );
          $operations[] = array('disc_migrate', array($type, $args));
        }

        break;

      // Migration type chooser.
      default:
        return array(
          '#theme' => 'item_list',
          '#items' => array(
            Link::createFromRoute('Courses', 'disc.batch', ['type' => 'dg_course']),
            Link::createFromRoute('Holes', 'disc.batch', ['type' => 'dg_hole']),
            Link::createFromRoute('Layouts', 'disc.batch', ['type' => 'dg_layout'])
          )
        );
        break;
    }

    if (empty($operations)) { return array('#markup' => 'No operations: ' . $type); }
    $batch = array(
      'title' => t('Exporting'),
      'operations' => $operations,
      'finished' => 'disc_migrate_finished_callback',
      'file' => drupal_get_path('module', 'disc') . '/disc.migrate.inc',
    );
    batch_set($batch);
    return batch_process('user');
  }
}