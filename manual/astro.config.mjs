// @ts-check
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';
import { typst } from 'astro-typst';

// https://astro.build/config
export default defineConfig({
	site: 'https://project-lemonlime.github.io',
	base: 'Project_LemonLime',
	integrations: [
		starlight({
			title: 'LemonLime Manual',
			tableOfContents: false,
			favicon: '/favicon.png',
			social: [{ icon: 'github', label: 'GitHub', href: 'https://github.com/Project-LemonLime/Project_LemonLime' }],
			sidebar: [
				{
					label: 'Manual',
					items: [
						{ label: '快速开始', slug: 'quickstart' },
						{ label: '简介', slug: 'intro' },
						{ label: '安装', slug: 'install' },
						{ label: '设置', slug: 'settings' },
						{ label: '比赛', slug: 'contest' },
						{ label: '测试', slug: 'judge' },
						{ label: '比赛统计', slug: 'statistics' },
						{ label: '常见问题及回答', slug: 'issue' },
					],
				},
			],
		}),
		typst({
			options: {
				remPx: 14,
			},
			target: "html",
		}),
	],
});
