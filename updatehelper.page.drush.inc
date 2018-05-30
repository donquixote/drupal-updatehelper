<?php

/**
 * Page callback for 'admin/reports/updates/drush'.
 *
 * @return array
 *
 * @see update_manager_update_form()
 */
function _updatehelper_drush_page() {

  $elements = [];

  $available = update_get_available(TRUE);
  if (empty($available)) {
    $elements['message'] = array(
      '#markup' => t('There was a problem getting update information. Try again later.'),
    );
    return $elements;
  }

  module_load_include('inc', 'update', 'update.compare');
  $project_data = update_calculate_project_data($available);

  /**
   * @var string[][][] $types
   *   Format: $[$type][$action][] = $project . '-' . $version
   *   with $type in 'security', 'unsupported', 'recommended'.
   *   with $action in 'unhack', 'update'.
   */
  $types = [
    'security' => [],
    'unsupported' => [],
    'recommended' => [],
  ];
  /**
   * @var array[][] $projects_by_type
   *   Format: $[$type][$project_name] = $project_info
   */
  $projects_by_type = [
    'security' => [],
    'unsupported' => [],
    'recommended' => [],
  ];
  foreach ($project_data as $projectname => $project) {
    // Filter out projects which are up to date already.
    if ($project['status'] == UPDATE_CURRENT) {
      continue;
    }
    if (empty($project['recommended'])) {
      // If we don't know what to recommend they upgrade to, we should skip
      // the project entirely.
      continue;
    }
    switch ($project['status']) {
      case UPDATE_NOT_SECURE:
      case UPDATE_REVOKED:
        $type = 'security';
        break;

      case UPDATE_NOT_SUPPORTED:
        $type = 'unsupported';
        break;

      case UPDATE_UNKNOWN:
      case UPDATE_NOT_FETCHED:
      case UPDATE_NOT_CHECKED:
      case UPDATE_NOT_CURRENT:
        $type = 'recommended';
        break;

      default:
        // Jump out of the switch and onto the next project in foreach.
        continue 2;
    }
    $types[$type]['unhack']['args'][$projectname] = $projectname . '-' . $project['existing_version'];
    $types[$type]['update']['args'][$projectname] = $projectname . '-' . $project['recommended'];
    $projects_by_type[$type][$projectname] = $project;
  }

  $root = DRUPAL_ROOT;
  $preparation = <<<EOT
cd $root
EOT;

  $elements['commands']['preparation'] = [
    '#type' => 'fieldset',
    '#title' => t('Preparation'),
    'command' => [
      '#markup' => '<pre>' . check_plain($preparation) . '</pre>',
    ],
  ];

  foreach (array_filter($projects_by_type) as $type => $type_projects) {

    $elements['commands'][$type] = [
      '#type' => 'fieldset',
      '#title' => $type,
    ];

    // Unhack.
    $dl_args = [];
    foreach ($type_projects as $projectname => $project) {
      $dl_args[] = $projectname . '-' . $project['existing_version'];
    }
    $command = 'drush dl -y ' . implode(' ', $dl_args);
    $elements['commands'][$type]['unhack'] = [
      '#theme_wrappers' => ['form_element'],
      '#title' => t('Remove local modifications, if any.'),
      '#markup' => '<pre>' . check_plain($command) . '</pre>',
    ];

    // Commit.
    foreach ($type_projects as $projectname => $project) {
      $command = ''
        . 'git add sites/all/modules/contrib/' . $projectname
        . "\n"
        . 'git commit -m"UNHACK ' . $projectname . '."';
      $elements['commands'][$type]['unhack_git'][$projectname] = [
        # '#theme_wrappers' => ['form_element'],
        # '#title' => t('Remove local modifications, if any.'),
        '#markup' => '<pre>' . check_plain($command) . '</pre>',
      ];
    }

    // Update.
    $dl_args = [];
    foreach ($type_projects as $projectname => $project) {
      $dl_args[] = $projectname . '-' . $project['recommended'];
    }
    $command = 'drush dl -y ' . implode(' ', $dl_args);
    $elements['commands'][$type]['update'] = [
      '#theme_wrappers' => ['form_element'],
      '#title' => t('Update to recommended version.'),
      '#markup' => '<pre>' . check_plain($command) . '</pre>',
    ];

    // Commit.
    foreach ($type_projects as $projectname => $project) {
      $command = ''
        . 'git add sites/all/modules/contrib/' . $projectname
        . "\n"
        . 'git commit -m"(up) ' . $projectname . ' ' . $project['existing_version'] . ' -> ' . $project['recommended'] . '."';
      $elements['commands'][$type]['update_git'][$projectname] = [
        # '#theme_wrappers' => ['form_element'],
        # '#title' => t('Remove local modifications, if any.'),
        '#markup' => '<pre>' . check_plain($command) . '</pre>',
      ];
    }
  }

  return $elements;
}

