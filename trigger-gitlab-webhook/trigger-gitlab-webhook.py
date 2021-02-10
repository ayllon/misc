#!/usr/bin/env python3
import json
import os
from argparse import ArgumentParser

import gitlab
import requests


def add_user_data(message, gl_user):
    message['user_id'] = gl_user.id
    message['user_name'] = gl_user.name
    message['user_avatar'] = gl_user.avatar_url


def visibility(visibility_str):
    if visibility_str == 'private':
        return 0
    elif visibility_str == 'internal':
        return 1
    else:
        return 2


def add_project_data(message, gl_project):
    message['project_id'] = gl_project.id
    message['project'] = {
        'id': gl_project.id,
        'name': gl_project.name,
        'description': gl_project.description,
        'web_url': gl_project.web_url,
        'avatar_url': None,
        'git_ssh_url': gl_project.ssh_url_to_repo,
        'git_http_url': gl_project.http_url_to_repo,
        'namespace': gl_project.namespace['name'],
        'visibility_level': visibility(gl_project.visibility),
        'path_with_namespace': gl_project.path_with_namespace,
        'default_branch': gl_project.default_branch,
        'homepage': gl_project.web_url,
        'url': gl_project.http_url_to_repo,
        'ssh_url': gl_project.ssh_url_to_repo,
        'http_url': gl_project.http_url_to_repo
    }
    message['repository'] = {
        'name': gl_project.name,
        'url': gl_project.http_url_to_repo,
        'description': gl_project.description,
        'homepage': gl_project.web_url,
        'git_http_url': gl_project.http_url_to_repo,
        'git_ssh_url': gl_project.ssh_url_to_repo,
        'visibility_level': visibility(gl_project.visibility)
    }


def add_branch_data(message, gl_branch):
    message['ref'] = 'refs/heads/' + gl_branch.name
    message['checkout_sha'] = gl_branch.commit['id']
    message['before'] = gl_branch.commit['parent_ids'][0]
    message['after'] = gl_branch.commit['id']
    message['commits'] = [
        {
            'id': gl_branch.commit['id'],
            'message': gl_branch.commit['message'],
            'title': gl_branch.commit['title'],
            'timestamp': gl_branch.commit['created_at'],
            'url': gl_branch.commit['web_url'],
            'author': {
                'name': gl_branch.commit['author_name'],
                'email': gl_branch.commit['author_email'],
            },
            'added': [],
            'modified': [],
            'removed': []
        }
    ]


def add_tag_data(message, gl_tag):
    message['ref'] = "refs/tags/" + gl_tag.name,
    message['before'] = '0000000000000000000000000000000000000000'
    message['after'] = gl_tag.commit['id']
    message['checkout_sha'] = gl_tag.commit['id']


# Parameters
parser = ArgumentParser()
parser.add_argument('--gitlab', type=str, default=os.environ.get('GITLAB'),
                    help='Gitlab server')
parser.add_argument('--gitlab-token', type=str, default=os.environ.get('GITLAB_TOKEN'),
                    help='Gitlab token')
parser.add_argument('--project', type=str, required=True,
                    help='Project name')

parser.add_argument('--repo-dir', type=str, default=os.getcwd(),
                    help='Repository')
parser.add_argument('--webhook', type=str, default=os.environ.get('GITLAB_WEBHOOK'),
                    help='Webhook URL')
parser.add_argument('--webhook-token', type=str, default=os.environ.get('GITLAB_WEBHOOK_TOKEN'),
                    help='Webhook secret token')
parser.add_argument('--tag', type=str, help='Tag')
parser.add_argument('--branch', type=str, help='Branch')

args = parser.parse_args()

# Connect to Gitlab
if not args.gitlab:
    parser.error('Gitlab endpoint required')
if not args.gitlab_token:
    parser.error('Gitlab token required')

gl = gitlab.Gitlab(args.gitlab, private_token=args.gitlab_token)
gl.auth()

# Obtain project information
gl_project = gl.projects.get(args.project)

if not args.branch and not args.tag:
    args.branch = gl_project.default_branch
    print(f'No branch or tag specified, using the default branch "{args.branch}"')

# Base message
message = {
    'object_kind': 'tag_push' if args.tag else 'push',
    'commits': [],
    'total_commits_count': 0
}

# Add user information
add_user_data(message, gl.user)

# Add project information
add_project_data(message, gl_project)

# Add commit information
if args.tag:
    add_tag_data(message, gl_project.tags.get(args.tag))
else:
    add_branch_data(message, gl_project.branches.get(args.branch))

# Send request
event_type = 'Tag Push Hook' if args.tag else 'Push Hook'
print(event_type)
print(json.dumps(message, indent=2))
resp = requests.post(
    args.webhook,
    headers={
        'X-Gitlab-Event': event_type,
        'X-Gitlab-Token': args.webhook_token
    },
    json=message
)
resp.raise_for_status()
print(resp.reason)
